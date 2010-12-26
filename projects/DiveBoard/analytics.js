Raphael.fn.drawGrid = function (x, y, w, h, wv, hv, color) {
    color = color || "#000";
    var path = ["M", Math.round(x) + .5, Math.round(y) + .5, "L", Math.round(x + w) + .5, Math.round(y) + .5, Math.round(x + w) + .5, Math.round(y + h) + .5, Math.round(x) + .5, Math.round(y + h) + .5, Math.round(x) + .5, Math.round(y) + .5],
        rowHeight = h / hv,
        columnWidth = w / wv;
    for (var i = 1; i < hv; i++) {
        path = path.concat(["M", Math.round(x) + .5, Math.round(y + i * rowHeight) + .5, "H", Math.round(x + w) + .5]);
    }
    for (i = 1; i < wv; i++) {
        path = path.concat(["M", Math.round(x + i * columnWidth) + .5, Math.round(y) + .5, "V", Math.round(y + h) + .5]);
    }
    return this.path(path.join(",")).attr({stroke: color});
};

Raphael.fn.drawVGridValues = function (x, y, w, h, dy, Y, color, txt) {
    color = color || "#000";
    var path = ["M", Math.round(x) + .5, Math.round(y) + .5, "L", Math.round(x + w) + .5, Math.round(y) + .5, Math.round(x + w) + .5, Math.round(y + h) + .5, Math.round(x) + .5, Math.round(y + h) + .5, Math.round(x) + .5, Math.round(y) + .5];
    for (var i = 1; i*Y < h; i += dy) {
        path = path.concat(["M", Math.round(x) + .5, Math.round(y + i*Y) + .5, "H", Math.round(x + w) + .5]);
        //text = this.text(20, y + i*Y, i).attr(txt).toBack();
    }
/*
    for (i = 1; i < wv; i++) {
        path = path.concat(["M", Math.round(x + i * columnWidth) + .5, Math.round(y) + .5, "V", Math.round(y + h) + .5]);
    }
*/
    return this.path(path.join(",")).attr({stroke: color});
};




function analytics(labels, data, div, width, height, display_labels) {
    function getAnchors(p1x, p1y, p2x, p2y, p3x, p3y) {
        var l1 = (p2x - p1x) / 2,
            l2 = (p3x - p2x) / 2,
            a = Math.atan((p2x - p1x) / Math.abs(p2y - p1y)),
            b = Math.atan((p3x - p2x) / Math.abs(p2y - p3y));
        a = p1y < p2y ? Math.PI - a : a;
        b = p3y < p2y ? Math.PI - b : b;
        var alpha = Math.PI / 2 - ((a + b) % (Math.PI * 2)) / 2,
            dx1 = l1 * Math.sin(alpha + a),
            dy1 = l1 * Math.cos(alpha + a),
            dx2 = l2 * Math.sin(alpha + b),
            dy2 = l2 * Math.cos(alpha + b);
        return {
            x1: p2x - dx1,
            y1: p2y + dy1,
            x2: p2x + dx2,
            y2: p2y + dy2
        };
    }
    
    // Draw
    //380x200 is medium, 800x300 is large
    //width = 800,
    //    height = 300,
    var     leftgutter = 30,
        bottomgutter = 20,
        topgutter = 20,
        stroke_width = 3,
        opacity_down = 0.3,
        opacity_up = 0.6,
        colorhue = .6 || Math.random(),
        color = "hsb(" + [colorhue, .5, 1] + ")",
        txt = {font: '12px Helvetica, Arial', fill: "#AAA"},
        txt1 = {font: '10px Helvetica, Arial', fill: "#fff"},
        txt2 = {font: '12px Helvetica, Arial', fill: "#000"},
        r = Raphael(div, width, height),
        X = (width - leftgutter) / labels.length,
        max = Math.max.apply(Math, data),
        Y = (height - bottomgutter - topgutter) / (max*1.04);
    
    //Draws the grid
    r.drawVGridValues (leftgutter + X * .5 + .5, topgutter + .5, width - leftgutter - X, height - topgutter - bottomgutter, 10, Y,  "#AAA", txt);

    //path is the main line
    //bgp is the blue below the line
    //label is the text of the floating label
    var path = r.path().attr({stroke: color, "stroke-width": stroke_width, "stroke-linejoin": "round"}),
        bgp = r.path().attr({stroke: "none", opacity: opacity_down, fill: color}),
        ugp = r.path().attr({stroke: "none", opacity: opacity_up, fill: color}),
        label = r.set(),
        is_label_visible = false,
        leave_timer,
        blanket = r.set();

    //creates the text of the floating label
    label.push(r.text(60, 12, "00 m").attr(txt));
    label.push(r.text(60, 27, "0.00").attr(txt1).attr({fill: color}));
    label.hide();

    //Creates the frame of the floating label
    var frame = r.popup(100, 100, label, "right").attr({fill: "#000", stroke: "#666", "stroke-width": 2, "fill-opacity": .7}).hide();

    //goes through all the data
    //p and bgpp are data-path
    var p, bgpp, ugpp;
    for (var i = 0, ii = labels.length; i < ii; i++) {
        var y = Math.round(topgutter + Y * data[i]),
            x = Math.round(leftgutter + X * (i + .5)),
            tx = "";

        if (display_labels && (!i || labels[i] != labels[i-1])) tx = r.text(x, height - 6, labels[i]).attr(txt).toBack();

        //first point, no line
        if (!i) {
            p = ["M", x, y, "C", x, y];
            bgpp = ["M", leftgutter + X * .5, height - bottomgutter, "L", x, y, "C", x, y];
            ugpp = ["M", leftgutter + X * .5, topgutter, "L", x, y, "C", x, y];
        }

        //next points, add to line
        if (i && i < ii - 1) {
            var Y0 = Math.round(topgutter + Y * data[i - 1]),
                X0 = Math.round(leftgutter + X * (i - .5)),
                Y2 = Math.round(topgutter + Y * data[i + 1]),
                X2 = Math.round(leftgutter + X * (i + 1.5));
            var a = getAnchors(X0, Y0, x, y, X2, Y2);
            p = p.concat([a.x1, a.y1, x, y, a.x2, a.y2]);
            bgpp = bgpp.concat([a.x1, a.y1, x, y, a.x2, a.y2]);
            ugpp = ugpp.concat([a.x1, a.y1, x, y, a.x2, a.y2]);
        }

        //create the dot
        var dot = r.circle(x, y, 4).attr({fill: "#000", stroke: color, "stroke-width": 2});
        dot.hide();

        blanket.push(r.rect(leftgutter + X * i, 0, X, height - bottomgutter).attr({stroke: "none", fill: "#fff", opacity: 0}));
        var rect = blanket[blanket.length - 1];

        //Gives the data to the floating label
        (function (x, y, data, lbl, dot) {
            var timer, i = 0;
            rect.hover(function () {
                clearTimeout(leave_timer);
                var side = "right";
                if (x + frame.getBBox().width > width) {
                    side = "left";
                }
                var ppp = r.popup(x, y, label, side, 1);
                frame.show().stop().animate({path: ppp.path}, 200 * is_label_visible);
                label[0].attr({text: data + " m" }).show().stop().animateWith(frame, {translation: [ppp.dx, ppp.dy]}, 200 * is_label_visible);
                label[1].attr({text: lbl + " min"}).show().stop().animateWith(frame, {translation: [ppp.dx, ppp.dy]}, 200 * is_label_visible);
                //dot.attr("r", 6);
                dot.show();
                is_label_visible = true;
            }, function () {
                //dot.attr("r", 4);
                dot.hide();
                leave_timer = setTimeout(function () {
                    frame.hide();
                    label[0].hide();
                    label[1].hide();
                    is_label_visible = false;
                }, 1);
            });
        })(x, y, data[i], labels[i], dot);
    }

    //Finishes the line at the bottom right color
    p = p.concat([x, y, x, y]);
    bgpp = bgpp.concat([x, y, x, y, "L", x, height - bottomgutter, "z"]);
    ugpp = ugpp.concat([x, y, x, y, "L", x, topgutter, "z"]);
    path.attr({path: p});
    bgp.attr({path: bgpp});
    ugp.attr({path: ugpp});

    //orders everything
    frame.toFront();
    label[0].toFront();
    label[1].toFront();
    blanket.toFront();
};
