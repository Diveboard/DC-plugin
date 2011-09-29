Name: diveboard-plugin 
Version: 1.1.0alpha1
Release: 1%{?dist}
Group: Applications/System
Summary: diveboard.com browser plugin for Linux
License: Unknown
URL: http://labs.vmware.com/flings/ioblazer
Source0: http://www.diveboard.com/plugin/latest/linux/diveboard.tgz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

%description
Explore scuba diving, log and share your experiences.

www.diveboard.com

%prep
%ifarch x86_64
%setup -n diveboard-plugin-1.1.0-alpha1-64
%else
%setup -n diveboard-plugin-1.1.0-alpha1
%endif

%install
%{__rm} -rf %{buildroot}
%{__mkdir_p} %{buildroot}/usr/lib/diveboard/
%{__install} -m 0755 usr/lib/diveboard/npDiveBoard.so %{buildroot}/usr/lib/diveboard/
%{__install} -m 0755 usr/lib/diveboard/libdivecomputer.so %{buildroot}/usr/lib/diveboard/

BROWSERS="mozilla firefox seamonkey"
for BROWSER in $BROWSERS; do
  %{__mkdir_p} %{buildroot}/usr/lib/$BROWSER/plugins
  %{__install} -m 0755 usr/lib/diveboard/npDiveBoard.so %{buildroot}/usr/lib/$BROWSER/plugins/
done

%clean
%{__rm} -rf %{buildroot}

%files
%defattr(-, root, root, 0755)
/usr/lib/*

%changelog
* Fri Sep 16 2011 Sergio Rubio <rubiojr@frameos.org> - 1.1.0alpha1-1
- Initial Release
