function graph(thermo) {
var f = thermo.temp;
var g = thermo.temp_outside;
var act = thermo.actuator;
var j = thermo.setpoint;
var cnv = document.getElementById('graph');
var gc = cnv.getContext('2d');
var width = 400;
var height = 300;
var lotemp = thermo.lotemp;
var hitemp = thermo.hitemp;
var xpix = width/f.length;
var ypix = height/(hitemp-lotemp);
gc.save();
gc.scale(1, -1);
gc.translate(0, -height);
var grad = gc.createLinearGradient(0, 0, 0, height);
grad.addColorStop(0, "rgb(0,0,255)");
grad.addColorStop(0.5, "rgb(0,255,0)");
grad.addColorStop(1, "rgb(255,0,0)");
gc.fillStyle = grad;
gc.fillRect(0,0,width,height);

// Outside temp graph
gc.lineWidth = 4;
gc.strokeStyle = "rgb(100,100,80)";
gc.beginPath();
for (i = 0; i < g.length; ++i) {
	var x = i*xpix;
	var y = (g[i]-lotemp)*ypix;
	if (!i)
		gc.moveTo(x, y);
	gc.lineTo(x+xpix/4, y);
	gc.lineTo(x+xpix*3/4, y);
}
gc.stroke();

// Setpoint graph
gc.lineWidth = 4;
gc.strokeStyle = "rgb(255,150,150)";
gc.beginPath();
for (i = 0; i < j.length; ++i) {
	var x = i*xpix;
	var y = (j[i]-lotemp)*ypix;
	if (!i)
		gc.moveTo(x, y);
	gc.lineTo(x+xpix/4, y);
	gc.lineTo(x+xpix*3/4, y);
}
gc.stroke();

// Inside temp graph
gc.lineWidth = 8;
gc.strokeStyle = "rgb(255,255,192)";
gc.beginPath();
for (i = 0; i < f.length; ++i) {
	var x = i*xpix;
	var y = (f[i]-lotemp)*ypix;
	if (!i)
		gc.moveTo(x, y);
	gc.lineTo(x+xpix/4, y);
	gc.lineTo(x+xpix*3/4, y);
}
gc.stroke();
gc.fillStyle = "rgba(255,255,0,0.25)";
// transparent yellow when heater actuator on
for (i = 0; i < act.length; ++i) {
	var x = i*xpix;
	if (act[i] > 0)
		gc.fillRect(x,20,xpix,20+height/4);
}
// transparent cyan when cooler actuator on
gc.fillStyle = "rgba(0,255,255,0.25)";
for (i = 0; i < act.length; ++i) {
	var x = i*xpix;
	if (act[i] < 0)
		gc.fillRect(x,20,xpix,20+height/4);
}
gc.restore();
gc.fillStyle = "rgb(0,0,0)";
gc.font = "20pt arial";
var heading = "Thermostat Time Series Data";
var met = gc.measureText(heading);
gc.fillText(heading, (width-met.width)/2, 30);
var footer = "Last " + thermo.interval + ", \u00B0F";
gc.font = "10pt arial";
gc.fillStyle = "rgba(255,255,255,0.75)";
met = gc.measureText(footer);
gc.fillText(footer, (width-met.width-20), height - 20);
gc.fillStyle = "rgba(0,0,0,0.75)";
for (i = lotemp+1; i < hitemp; ++i) {
	if (!(i % 5))
		gc.fillText(i, 10, height - (i-lotemp)*ypix + 5);
	
}
// Legend:
gc.fillStyle = "rgba(0,0,0,0.5)";
gc.fillRect(25, height - 60, 120, 58);
gc.fillStyle = "rgba(255,255,255,0.75)";
gc.lineWidth = 4;
gc.strokeStyle = "rgb(100,100,80)";
gc.beginPath();
gc.moveTo(30, height-10);
gc.lineTo(50, height-10);
gc.stroke();
gc.fillText("Outside temp.", 55, height - 5);
gc.strokeStyle = "rgb(255,150,150)";
gc.beginPath();
gc.moveTo(30, height-30);
gc.lineTo(50, height-30);
gc.stroke();
gc.fillText("Set point", 55, height - 25);
gc.lineWidth = 8;
gc.strokeStyle = "rgb(255,255,192)";
gc.beginPath();
gc.moveTo(30, height-50);
gc.lineTo(50, height-50);
gc.stroke();
gc.fillText("Inside temp.", 55, height - 45);
} // function graph()


