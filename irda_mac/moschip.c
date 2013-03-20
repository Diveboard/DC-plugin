#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <IOKit/IOKitLib.h>
#include <IOKit/usb/IOUSBLib.h>
#include <CoreFoundation/CFString.h>

#include "irphy.h"


#define MAGIC_OPEN    0x20085 //O_NOCTTY|O_RDWR|SYNC //0x20005
#define MAGIC_IOCTL   0x2000740d
#define SERVICE_CLASS "AppleIrDA"
#define dev_name "/dev/cu.IrDA-IrCOMM0"
#define STEP_WAIT     10000 //microseconds
#define IRLAP_FCS_INIT          0xFF  
#define IRLAP_EOF           0xC1  
#define IRLAP_BOF           0xC0  
#define IRLAP_BOF_LEN 10

//#define kMyDriversIOKitClassName  kIOUSBDeviceClassName

static int dev_fd = -1;
static io_connect_t  dataPort;
static uint8_t read_buffer[2048];
static size_t read_buffer_count = 0;
static size_t read_buffer_pos = 0;


bool getVidAndPid(io_service_t device, int *vid, int *pid)
{
  bool success = false;

  CFNumberRef cfVendorId = (CFNumberRef)IORegistryEntryCreateCFProperty(device, CFSTR("idVendor"), kCFAllocatorDefault, 0);
  if (cfVendorId && (CFGetTypeID(cfVendorId) == CFNumberGetTypeID()))
  {
    Boolean result;
    result = CFNumberGetValue(cfVendorId, kCFNumberSInt32Type, vid);
    CFRelease(cfVendorId);
    if (result)
    {
      CFNumberRef cfProductId = (CFNumberRef)IORegistryEntryCreateCFProperty(device, CFSTR("idProduct"), kCFAllocatorDefault, 0);
      if (cfProductId && (CFGetTypeID(cfProductId) == CFNumberGetTypeID()))
      {
        Boolean result;
        result = CFNumberGetValue(cfProductId, kCFNumberSInt32Type, pid);
        CFRelease(cfProductId);
        if (result)
        {
          success = true;
        }
      }
    }
  }

  return (success);
}



void get_bauds()
{
    kern_return_t kernResult; 
    char inbuff[258] = "!";
    size_t isize= 1;
    char outbuff[1024];
    size_t osize= 4;

    kernResult = IOConnectCallStructMethod(dataPort, 0x0,
                           // inputStructure
                            inbuff, isize,
                            // ouputStructure
                            outbuff, &osize );

    if (kernResult != KERN_SUCCESS) {
        printf("IOConnectCallStructMethod returned %x\n", kernResult);
        exit(1);
    }
    printf("get_bauds done\n");
}

uint8_t irphy_set_baud(speed_t baud)
{
  kern_return_t kernResult; 
  uint8_t inbuff[7] = {0x18, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
  switch(baud){
    case B2400:    inbuff[1]=0x01; break;
    case B9600:    inbuff[1]=0x01; break;
    case B19200:   inbuff[1]=0x02; break;
    case B38400:   inbuff[1]=0x03; break;
    case B57600:   inbuff[1]=0x04; break;
    case B115200:  inbuff[1]=0x05; break;
  /*case B576000:  inbuff[1]=0x06; break;
    case B1152000: inbuff[1]=0x07; break;
    case B4000000: inbuff[1]=0x08; break;
    */
  }
  size_t isize= 7;
  char outbuff[1024];
  size_t osize= 0;

  kernResult = IOConnectCallStructMethod(dataPort, 0x0,
                         // inputStructure
                          inbuff, isize,
                          // ouputStructure
                          outbuff, &osize );

  if (kernResult != KERN_SUCCESS) {
      printf("IOConnectCallStructMethod returned %x\n", kernResult);
      exit(1);
  }
  printf("set baud done\n");
}



void unknown_call()
{
    kern_return_t kernResult; 
    char inbuff[8] = { 0x18 , 0x01 , 0x00,  0x00,  0x00 , 0x00 , 0x00, 0x00};
    size_t isize= 7;
    char outbuff[1024];
    size_t osize= 0;

    kernResult = IOConnectCallStructMethod(dataPort, 0x0,
                           // inputStructure
                            inbuff, isize,
                            // ouputStructure
                            outbuff, &osize );

    if (kernResult != KERN_SUCCESS) {
        printf("IOConnectCallStructMethod returned %x\n", kernResult);
        exit(1);
    }
    printf("unknown_call done\n");
}




void irphy_reset(){
  kern_return_t kernResult; 
  mach_port_t   masterPort;
  io_service_t  serviceObject;
  io_service_t irdaServiceObject;
  io_iterator_t   iterator;
  CFDictionaryRef classToMatch;


  if(dev_fd>=0) {
    IOServiceClose(irdaServiceObject);
    close(dev_fd);  
  }

  //First we need to open the device and keep it open ! or else it makes the OS crash....
  dev_fd = open(dev_name, MAGIC_OPEN);
  if (dev_fd<=0) {
    fprintf(stderr, "open '%s' failed\n", dev_name);
    exit(1);    
  }

  if (ioctl(dev_fd, MAGIC_IOCTL) == -1) {
    fprintf(stderr, "ioctl failed\n");
    exit(1);    
  }

  tcflush(dev_fd, TCIOFLUSH);

  //Get the Master port
  kernResult = IOMasterPort(MACH_PORT_NULL, &masterPort);
  if (kernResult != KERN_SUCCESS) {
    printf( "IOMasterPort returned %d\n", kernResult);
    exit(1);
  }

  //Now iterate through the available services until we find one for AppleIrDA
  IORegistryCreateIterator(masterPort,
    kIOServicePlane,
    kIORegistryIterateRecursively,
    &iterator);

  while (serviceObject = IOIteratorNext(iterator)) {
    if (IOObjectConformsTo(serviceObject, SERVICE_CLASS)) {
      printf("FOUND\n");
      irdaServiceObject=serviceObject;
    }
  }
  IOObjectRelease(iterator);
  if (!irdaServiceObject)
  {
    printf("Couldn't find any matches for irda.\n");
    exit(1);
  }

  // This call will cause the user client to be instantiated.
  kernResult = IOServiceOpen(irdaServiceObject, mach_task_self(), 123, &dataPort);
  IOObjectRelease(irdaServiceObject);
  if (kernResult != KERN_SUCCESS) {
      printf("IOServiceOpen returned %x\n", kernResult);
      exit(1);
  }

  irphy_set_baud(B9600);
}


void irphy_send_frame(unsigned char *data, uint16_t size){  
  if (!write(dev_fd, data, size)) {
    printf("ERROR: write error\n");
    exit(1);
  }
}


uint8_t irphy_wait(int16_t ms_timeout){
  unsigned long wait = 0;
  unsigned long us_timeout = ms_timeout * 100;
  while(read_buffer_count == 0){
    receive_char();
    if (read_buffer_count > 0) break;
    usleep(STEP_WAIT);
    if (wait > us_timeout)
      return false;
    wait += STEP_WAIT;
  }
  return true;
}

uint8_t irphy_receive(){
  if (read_buffer_count == 0) {
    printf("ERROR: Read data is empty !!!\n");
    exit(2);
  }
  uint8_t ret;
  ret = read_buffer[read_buffer_pos];
  //restore buffer to emptyness
  if (++read_buffer_pos >= read_buffer_count) {
    read_buffer_count = 0;
    read_buffer_pos = 0;
  }
  return(ret);
}

int receive_char() {
  kern_return_t kernResult; 
  uint8_t inbuff[2048];
  uint8_t read_buffer2[2048];
  size_t isize= sizeof(inbuff);
  size_t osize= sizeof(read_buffer);
  size_t out_size;

  memset(inbuff,0x0, sizeof(inbuff));
  inbuff[0] = '#';

  usleep(5000);

  kernResult = IOConnectCallStructMethod(dataPort, 0x0,
                         // inputStructure
                          inbuff, isize,
                          // ouputStructure
                          read_buffer2, &osize );

  if (kernResult != KERN_SUCCESS) {
      printf("IOConnectCallStructMethod returned %x in receive_char\n", kernResult);
      exit(1);
      return(-1);
  }

  if (osize>0) {
    //printf("RECEIVED %lu bytes\n", osize);
    uint8_t fcs0, fcs1, v;  
    size_t i,j;
    out_size = osize;

    //reconstructing the frame for IRLAP....
    fcs0=IRLAP_FCS_INIT;  
    fcs1=IRLAP_FCS_INIT;
    printf("[");
    for (i=0; i<osize; i++) {
      v = read_buffer2[i];
      printf("%x ", v);
      v^=fcs0;  
      fcs0=fcs1;  
      fcs1=v;  
    
      fcs1^=fcs1<<4;  
      fcs0^=fcs1>>4;  
      fcs0^=fcs1<<3;  
      fcs1^=fcs1>>5;  
    }
    printf("] (%lu)\n", osize);

    read_buffer2[osize+0] = ~fcs0;
    read_buffer2[osize+1] = ~fcs1;
    out_size += 2;

    //Escape flags

    for (i=0,j=0; i< osize+2; i++){
      if (read_buffer2[i]==0xc0 || read_buffer2[i]==0xc1 || read_buffer2[i]==0x7d){
        read_buffer[IRLAP_BOF_LEN+j++] = 0x7d;
        read_buffer[IRLAP_BOF_LEN+j++] = read_buffer2[i] ^ 0x20;
        out_size++;
      } 
      else
        read_buffer[IRLAP_BOF_LEN+j++] = read_buffer2[i];
    }

    memset(read_buffer, IRLAP_BOF, IRLAP_BOF_LEN);
    read_buffer[out_size+IRLAP_BOF_LEN] = IRLAP_EOF;

    //Setting up the buffer cursor
    read_buffer_count = out_size+IRLAP_BOF_LEN+1;
    read_buffer_pos = 0;
    return(1);
  }

  return(0);
}




