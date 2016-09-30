/**************************************************************************************************************
* Copyright (c) 2012, Goincompany.com
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the DRINKS Toolkit nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY Goincompany.com ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL Goincompany.com BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 To get sources and documentation, please visit:

    http://www.goincompany.com/drinks.php

***************************************************************************************************************/
function Gauge(element){

  Gauge.inherits(AnalogOutput);
  AnalogOutput.call(this, element);
  //radius of the element
  var pen = this.pen;
  this.radius = element.getAttribute("radius") || 100;
  this.rx = this.radius/100;
  this.setWidth(this.radius*2+36*this.rx);
  this.setHeight(this.radius*2+36*this.rx);
  var main_grd, grd, grd_text, grd2, grd3;
  var createGradients = function(){
    main_grd = pen.createLinearGradient(-this.radius/2, -this.radius/2, this.radius/2, 0);
    main_grd.addColorStop(0, '#bbb');
    main_grd.addColorStop(0.5, '#eee');
    main_grd.addColorStop(1, '#bbb');
    grd2 = pen.createRadialGradient(0, 0, this.radius, 0, 0, this.radius+10*this.rx);
    grd2.addColorStop(0, '#eee');
    grd2.addColorStop(0.1, '#bbb');
    grd2.addColorStop(1, '#eee');
    grd3 = pen.createRadialGradient(0, 0, this.radius, 0, 0, this.radius+15*this.rx);
    grd3.addColorStop(0, '#bbb');
    grd3.addColorStop(0.1, '#555');
    grd3.addColorStop(1, '#bbb');
    grd = pen.createRadialGradient(-(3*this.rx),-(7.5*this.rx), 0, (3*this.rx), (7.5*this.rx), 50*this.rx);
    grd.addColorStop(0, '#aaa');
    grd.addColorStop(0.4, 'black');
    grd_text = pen.createLinearGradient(-pen.measureText(this.text).width, -pen.measureText(this.text).width, pen.measureText(this.text).width, 0);
    grd_text.addColorStop(0, '#333');
    grd_text.addColorStop(0.5, '#777');
    grd_text.addColorStop(1, '#333');
  }.bind(this);
  createGradients();
  this.divisions = element.getAttribute("divisions") || 100;
  this.dx = 270/this.divisions;
  var precision = this.precision;
  this.color = element.getAttribute("color") || "red";
  

  this.drawRange = function(){
    var beg = Math.min (this.min_range, this.max_range);
    var end = Math.max (this.min_range, this.max_range);
    
    if(this.range_from!=null && this.range_to!=null && this.range_from>=beg && this.range_to<=end){
      pen.save();
      pen.strokeStyle="rgba(255, 0, 0 , 0.5)";
      pen.lineWidth=10*this.rx;
      pen.beginPath();
      var range_min_position = (parseFloat(this.range_from)-this.min_range)*this.divisions/(this.max_range-this.min_range);
      var range_max_position = (parseFloat(this.range_to)-this.min_range)*this.divisions/(this.max_range-this.min_range);
      beg = Math.min (range_min_position, range_max_position);
      end = Math.max (range_min_position, range_max_position);
      pen.arc(0, 0, 60*this.rx, (3*Math.PI/4)+beg * (this.dx)*Math.PI / 180, (3*Math.PI/4)+end * (this.dx)*Math.PI / 180, false);
      pen.stroke();
      pen.closePath();
      pen.restore();
    }  
  }

  this.drawGrid = function(){
    var i, inset;
    var num;
    var divfactor = 100/this.divisions>1?1:100/this.divisions;
    pen.font='bolder'+' '+(this.radius*divfactor*0.15)+'px'+' '+this.fontFamily;
    precision = this.precision;
    for(var i = 0; i <= this.divisions; i++){
      var cosfact = Math.cos((-Math.PI/4)+i*this.dx*Math.PI / 180);
      var sinfact = Math.sin((-Math.PI/4)+i*this.dx*Math.PI / 180);
      if(i%5==0 || this.divisions/10<1){
        if(i%10==0 || this.divisions/10<1){
          inset = 10 * this.rx;
          pen.lineWidth=1.5 * this.rx * divfactor;
          pen.fillStyle=this.color;
          num = Math.round((this.min_range+(this.max_range-this.min_range)*i/this.divisions)*precision)/precision; 
          pen.fillText(num,-(4*this.radius/5-pen.measureText(num).width/4) * cosfact - pen.measureText(num).width/2, -(4*this.radius/5) * sinfact + (this.radius/(20*this.dx)*divfactor));
        } else {
          inset = 8 * this.rx;
          pen.lineWidth= 1 * this.rx * divfactor;
        }
      } else {
        inset = 5 * this.rx;
        pen.lineWidth=0.5 * this.rx * divfactor;
      }

      pen.strokeStyle='black';      
      pen.beginPath();
      pen.moveTo((this.radius - inset) * cosfact, -(this.radius - inset) * sinfact);
      pen.lineTo(this.radius * cosfact, -this.radius * sinfact);
      pen.stroke();
      pen.closePath();         
    }
  }

  this.drawValue = function(){
    var x1 = Math.cos ((-Math.PI*7/20)+this.prec_position * this.dx * Math.PI / 180);
    var y1 = Math.sin ((-Math.PI*7/20)+this.prec_position * this.dx * Math.PI / 180);
    var x2 = Math.cos ((-Math.PI*3/20)+this.prec_position * this.dx *Math.PI / 180);
    var y2 = Math.sin ((-Math.PI*3/20)+this.prec_position * this.dx *Math.PI / 180);
    var grd_val = pen.createLinearGradient(-(this.radius/2) * x1, -(this.radius/2) * y1,-(this.radius/2) * x2, -(this.radius/2) * y2);
    pen.shadowOffsetX = 2*this.rx;
    pen.shadowOffsetY = 2*this.rx;
    pen.shadowColor = "#999";
    pen.shadowBlur = 3*this.rx;
    grd_val.addColorStop(0, 'white');
    grd_val.addColorStop(1, this.color);
    pen.fillStyle=grd_val;
    pen.strokeStyle="#777";
    pen.moveTo(-15*this.rx*x1, -15*this.rx* y1);    
    pen.lineTo(-(90*this.rx) * Math.cos ((-Math.PI/4)+this.prec_position * (this.dx)*Math.PI / 180),-(90*this.rx) * Math.sin ((-Math.PI/4)+this.prec_position * (this.dx)*Math.PI / 180));
    pen.lineTo(-15*this.rx*x2, -15*this.rx* y2);
    pen.stroke();
    pen.fill();  
    pen.closePath();
    pen.fillStyle=grd;
    pen.beginPath();
    pen.arc(0, 0, 15*this.rx, (Math.PI/180)*0, (Math.PI/180)*360, false);
    pen.closePath();
    pen.fill();
  }

  this.__defineSetter__("radius", function(radius){
    this._radius = radius;
    this.rx = radius/100;
    this.setHeight(radius*2+36*this.rx);
    this.setWidth(radius*2+36*this.rx);
    if(this.radius)
      createGradients();
  });

  this.__defineGetter__("radius", function(){
    return this._radius;
   });

  this.__defineSetter__("divisions", function(divisions){
    this._divisions = divisions;
    this.dx = 270/this.divisions;
    precision = this.precision;
  });

  this.__defineGetter__("divisions", function(){
    return this._divisions;
   });

  this.radius=parseInt(element.getAttribute("radius")) || 100;
  this.divisions = element.getAttribute("divisions") || 100;

  
  this.render = function(){
    this.canvas.width = this.canvas.width;
    this.analogOutputCommonOperations();
    pen.save();
    pen.translate(this.radius+18*this.rx, this.radius+18*this.rx);
    pen.beginPath();
    pen.fillStyle=grd3;
    pen.shadowOffsetX = 0;
    pen.shadowOffsetY = 0;
    pen.shadowColor = "#999";
    pen.shadowBlur = 3*this.rx;
    pen.strokeStyle="#aaa";
    pen.arc(0, 0, this.radius+16*this.rx, (Math.PI/180)*360, (Math.PI/180)*0, false);
    pen.closePath();
    pen.stroke();
    pen.fill();
    pen.shadowColor = "transparent";
    pen.beginPath();
    pen.fillStyle=grd2;
    pen.strokeStyle="#777";
    pen.arc(0, 0, this.radius+7*this.rx, (Math.PI/180)*360, (Math.PI/180)*0, false);
    pen.closePath();
    pen.fill();
    pen.stroke();
    pen.strokeStyle=grd2;
    pen.beginPath();
    pen.fillStyle=main_grd;
    pen.arc(0, 0, this.radius, (Math.PI/180)*360, (Math.PI/180)*0, false);
    pen.closePath();
    pen.fill();
    pen.stroke();
    if(!this.hide_grid)
      this.drawGrid();
    this.drawRange();
    pen.fillStyle=grd_text;
    pen.font='italic'+'  '+20*this.rx+'px'+' '+this.fontFamily;
    if(pen.measureText(this.text).width>2/3*this.radius)
      pen.font='italic'+'  '+(3/2*this.radius)/this.text.length+'px'+' '+this.fontFamily;
    pen.fillText(this.text, -pen.measureText(this.text).width/2, -this.radius/3);
    pen.beginPath();
    pen.strokeStyle='rgba(255, 0, 0, 0.7)';
    pen.lineWidth=1;
    pen.fillStyle="red";
    this.drawValue();
    pen.restore();
  }
}

function VuMeter(element){

  VuMeter.inherits(AnalogOutput);
  AnalogOutput.call(this, element);
  var pen = this.pen;
  if(this.width==null)  
    this.setWidth(300);
  if(this.height==null)  
    this.setHeight(180);
  this.wx = this.width/300;
  this.hx = this.height/180;
  var grd, grd2;
  
  var createGradients = function(){
    grd =  pen.createLinearGradient(-10*this.wx, 0, this.width+10*this.wx, 0);
    grd.addColorStop(0, '#999');
    grd.addColorStop(0.1, '#333');
    grd.addColorStop(0.5, '#999');
    grd.addColorStop(0.9, "#333");
    grd.addColorStop(1, "#999");
    grd2 =  pen.createLinearGradient(-5*this.wx, this.height/2-10*this.hx, 5*this.wx, this.height/2-15*this.hx);
    grd2.addColorStop(0, '#CCCC33');
    grd2.addColorStop(0.3, 'white');
    grd2.addColorStop(0.5, '#CCCC33');
  }.bind(this);
  createGradients();

  this.divisions = element.getAttribute("divisions") || 100.00;;
  this.dx = 100/this.divisions;
  var precision = this.precision;
  this.drawRange = function(){
    var beg = Math.min (this.min_range, this.max_range);
    var end = Math.max (this.min_range, this.max_range);
    if(this.range_from!=null && this.range_to!=null && this.range_from>=beg && this.range_to<=end){
      pen.save();
      pen.strokeStyle="rgba(255, 0, 0 , 0.5)";
      pen.lineWidth=8*this.wx;
      pen.beginPath();
      var range_min_position = (parseFloat(this.range_from)-this.min_range)*this.divisions/(this.max_range-this.min_range);
      var range_max_position = (parseFloat(this.range_to)-this.min_range)*this.divisions/(this.max_range-this.min_range);    
      beg = Math.min (range_min_position, range_max_position);
      end = Math.max (range_min_position, range_max_position);
      pen.arc(0, this.height/2+10*this.hx, this.width/2-10*this.wx, (220+beg)*Math.PI/180, (220+end)*Math.PI/180, false);
      pen.stroke();
      pen.closePath();
      pen.restore();
    }  
  }

  this.drawGrid = function(){
    var i, inset;
    var num;
    var divfactor = 100/this.divisions>1?1:100/this.divisions;
    pen.font='italic'+' '+'bolder'+' '+((10*this.wx)*(divfactor))+'px'+' '+this.fontFamily;
    precision = this.precision;
    for(i = 0; i <= this.divisions; i++){
      var y = Math.round((this.height/2+10*this.hx+Math.sin((220+i*this.dx)*Math.PI/180)*this.width/2)*100)/100;
      var x = Math.round((Math.cos((220+i*this.dx)*Math.PI/180)*this.width/2)*100)/100;
      if(i % 5 == 0 || this.divisions/10<1){
        if(i % 10 == 0 || this.divisions/10<1){
          inset = 0.08 * this.width/2;
          pen.lineWidth=1.5*this.wx;
          num = Math.round((this.min_range+(this.max_range-this.min_range)*i/this.divisions)*precision)/precision;  
          pen.fillStyle='#333';
        } else {
          inset = 0.05 * this.width/2;
          pen.lineWidth=1*this.wx;
        }
      } else {
        inset = 0.03 * this.width/2;
        pen.lineWidth=0.5*this.wx;
      }
      var y1 = Math.round((this.height/2+10*this.hx+Math.sin((220+i*this.dx)*Math.PI/180)*(this.width/2+inset))*100)/100;
      var x1 = Math.round((Math.cos((220+i*this.dx)*Math.PI/180)*(this.width/2+inset))*100)/100;  
      pen.strokeStyle='#333';      
      pen.beginPath();
      pen.moveTo(x, y);
      pen.lineTo(x1, y1);
      pen.stroke();
      pen.closePath();         
      if(i % 10 == 0 || this.divisions/10<1)
        pen.fillText(num, x1-pen.measureText(num).width/2, y1-5*this.hx);

    }
  }

  this.drawValue = function(){
    pen.beginPath();
    pen.strokeStyle="rgba(255, 0, 0, 0.7)";
    pen.lineWidth=2*this.wx;
    pen.moveTo(0, this.height/2-10*this.hx);
    var y1 = Math.round((this.height/2+10*this.hx+Math.sin((220+this.prec_position*this.dx)*Math.PI/180)*(this.width/2+5*this.wx))*100)/100;
    var x1 = Math.round((Math.cos((220+this.prec_position*this.dx)*Math.PI/180)*(this.width/2+5*this.wx))*100)/100;  
    pen.lineTo(x1, y1);
    pen.stroke();
    pen.closePath();
  }

  this.__defineSetter__("width", function(width){
    this._width = width;
    this.canvas.width=width;
    this.container.style.width = width+'px';
    this.wx= width/300;
    if(this._height)
      createGradients();
  });
  
  this.__defineGetter__("width", function(){
    return this._width;
   });

  this.__defineSetter__("height", function(height){
    this._height = this.width*180/300;
    if(height>this.width*180/300){
      this.canvas.height=height;
      this.container.style.height = height+'px';
    }else{
      this.canvas.height=this._height;
      this.container.style.height = this._height+'px';
    }
    this.hx= this._height/180;
    if(this.height && this.width)
      createGradients();
  });
  
  this.__defineGetter__("height", function(){
    return this._height;
   });

  this.__defineSetter__("divisions", function(divisions){
    this._divisions = divisions;
    this.dx = 100/this.divisions;
    precision = this.precision;
  });

  this.__defineGetter__("divisions", function(){
    return this._divisions;
   });


  this.width = this.canvas.width;
  this.height = this.canvas.height;
  this.divisions = element.getAttribute("divisions") || 100;

  this.render = function(){
    this.canvas.width = this.canvas.width;
    this.analogOutputCommonOperations();
    pen.fillStyle=grd;
    pen.fillRect(0, 0, this.width, this.height);
    pen.save();
    pen.shadowOffsetX = -2*this.wx;
    pen.shadowOffsetY = -2*this.hx;
    pen.shadowColor = "#333";
    pen.shadowBlur = 5*this.wx;
    pen.beginPath();
    pen.strokeStyle=grd;
    pen.lineWidth=5*this.wx;
    pen.moveTo(10*this.wx, 10*this.hx);
    pen.lineTo(this.width-10*this.wx, 10*this.hx);
    pen.lineTo(this.width-10*this.wx, this.height-10*this.hx);
    pen.lineTo(10*this.wx, this.height-10*this.hx);
    pen.closePath();  
    pen.fillStyle="white";
    pen.stroke();
    pen.fill();
    pen.restore();
    pen.save();
    pen.translate(this.width/2, this.height/2);
    pen.beginPath();
    pen.arc(0, this.height/2+10*this.hx, this.width/2, 220*Math.PI/180, 320*Math.PI/180, false);
    pen.stroke();
    pen.closePath();
    this.drawRange();
    pen.fillStyle="#222";
    pen.shadowOffsetX = 0;
    pen.shadowOffsetY = 0*this.hx;
    pen.shadowColor = "#555";
    pen.shadowBlur = 3*this.wx;
    pen.font='italic'+'  '+20*this.wx+'px'+' '+this.fontFamily;
    if(pen.measureText(this.text).width>this.width/2)
      pen.font='italic'+'  '+(this.width/2)/this.text.length+'px'+' '+this.fontFamily;
    pen.fillText(this.text, -pen.measureText(this.text).width/2, this.height/3 /*-this.width/4*/);
    pen.shadowOffsetX = 0*this.wx;
    pen.shadowOffsetY = 0*this.hx;
    pen.shadowColor = "#333";
    pen.shadowBlur = 2*this.wx;
    if(!this.hide_grid)
      this.drawGrid();
    pen.shadowOffsetX = 1*this.wx;
    pen.shadowOffsetY = 1*this.hx;
    pen.shadowColor = "#333";
    pen.shadowBlur = 2*this.wx;
    this.drawValue();
    pen.shadowColor = "transparent";
    pen.fillStyle=grd;
    pen.strokeStyle=grd;
    pen.lineWidth=4*this.wx;
    pen.beginPath();
    pen.arc(0, this.height/2, 20*this.wx, Math.PI, 0, false);
    pen.stroke();
    pen.fill();
    pen.closePath();
    pen.lineWidth=1*this.wx;
    pen.fillStyle=grd2;
    pen.beginPath();
    pen.arc(0, this.height/2-10*this.hx, 5*this.wx, 2*Math.PI, 0, false);
    pen.stroke();
    pen.fill();
    pen.closePath();
    pen.restore();
  }

}

function dMeter(element){

  dMeter.inherits(DigitalOutput);
  DigitalOutput.call(this, element);
  var numbers = {"0":"0111111","1":"0100001", "2":"1110110", "3":"1110011", "4":"1101001", "5":"1011011","6":"1011111", "7":"0110001","8":"1111111","9":"1111011"};
  var pen = this.pen;
  if(this.width==null)
    this.setWidth(100*this.cipher);    
  if(this.height==null)
    this.setHeight(95*this.cipher);
  var comma = null;
  var negative = false;
  var width = (this.width-20*this.width/150)/this.cipher;
  var height = this.height/this.cipher;
  var hx = height/75;
  var wx = width/100;
  var ht = this.height/30;
  var cx = width*1/3;

  var grd;
  var createGradients = function(){
    grd = pen.createLinearGradient(0, 0, 0, this.height);
    grd.addColorStop(0, '#bbb');
    grd.addColorStop(0.5, '#eee');
    grd.addColorStop(1, '#bbb');  
  }.bind(this);
  createGradients();

  this.__defineSetter__("width", function(w){
    this._width = w;
    this.canvas.width = w;
    this.container.style.width = w+'px';
    width = (w-20*this.width/150)/this.cipher;
    wx = width/100;
    cx = width*1/3;
  });

  this.__defineGetter__("width", function(){
    return this._width;
  });

  this.__defineSetter__("height", function(h){
    this._height = h;
    this.canvas.height = h;
    this.container.style.height = h+'px';
    height = h/this.cipher;
    hx = height/75; 
    if(this.height)
      createGradients();
    });

  this.__defineGetter__("height", function(){
    return this._height;
  });

  this.__defineSetter__("color", function(color){
    if(color!="auto")
      this._color = color;
    else
      this._color = '#333';
  });  

  this.__defineGetter__("color", function(){
    return this._color;
  });

  this.__defineSetter__("bgcolor", function(bgcolor){
    if(bgcolor!="auto")
      this._bgcolor = bgcolor;
    else
      this._bgcolor = grd;
  });

  this.__defineGetter__("bgcolor", function(){
    return this._bgcolor;
  });

  this.__defineSetter__("cipher", function(cipher){
    this._cipher = cipher;
    width = (this.width-20*this.width/150)/cipher;
    height = this.height/cipher;
    wx = width/100;
    hx = height/75; 
    cx = width*1/3;
  });

  this.__defineGetter__("cipher", function(){
    return this._cipher;
  });

  this.bgcolor = element.getAttribute("bgcolor") || grd;
  this.color = element.getAttribute("color") || "#333";
  this.cipher=parseInt(element.getAttribute('cipher')) || 2;

  var lightLed =function(num, index){
    if(numbers[num][index]==1){
      pen.fillStyle=this.color;
    }
    else
      pen.fillStyle="transparent";
  }.bind(this);

  this.width = this.canvas.width;
  this.height = this.canvas.height;
  var writeCipher = function(num, c){

    var cip = 40*this.width/150+width*c;
    pen.save();    
    pen.translate(cip/2, this.height/2);
    if((c-1)/2==comma-1){
      pen.beginPath();
      pen.fillStyle=this.color;
      pen.arc(width/2, width/10+cx, height/10, (Math.PI/180)*0, (Math.PI/180)*360, false);
      pen.fill();    
      pen.closePath();
    }        
    pen.beginPath();
    lightLed(num, 0);
    pen.moveTo(-width/4+(2*wx), 0);
    pen.lineTo(-width/6+(2*wx), -ht);
    pen.lineTo(width/6+(2*wx), -ht);
    pen.lineTo(width/4+(2*wx), 0);
    pen.lineTo(width/6+(2*wx), ht);
    pen.lineTo(-width/6+(2*wx), ht);
    pen.lineTo(-width/4+(2*wx), 0);
    pen.fill();
    pen.closePath();
    pen.beginPath();
    lightLed(num,1);
    pen.moveTo(width/4, (-2*hx));
    pen.lineTo(width/6+(2*wx), (-2*hx)-ht);
    pen.lineTo(width/6+(4*wx), (-2*hx)-ht-cx);
    pen.lineTo(width/4+(4*wx), (-2*hx)-2*ht-cx);
    pen.lineTo(width/4+(2*wx), (-2*hx));
    pen.fill();
    pen.closePath();
    pen.beginPath();
    lightLed(num, 2);
    pen.moveTo(width/4+(4*wx), (-4*hx)-2*ht-cx);
    pen.lineTo(width/6+(4*wx), (-4*hx)-ht-cx);
    pen.lineTo(-width/6+(4*wx), (-4*hx)-ht-cx);
    pen.lineTo(-width/4+(4*wx), (-6*hx)-2*ht-cx);
    pen.lineTo(width/4+(4*wx), (-6*hx)-2*ht-cx);
    pen.fill();
    pen.closePath();
    pen.beginPath();
    lightLed(num, 3);
    pen.moveTo(-width/4+(4*wx), (-2*hx)-2*ht-cx);
    pen.lineTo(-width/6+(4*wx), (-2*hx)-ht-cx);
    pen.lineTo(-width/6+(2*wx), (-2*hx)-ht);
    pen.lineTo(-width/4+(2*wx), (-2*hx));
    pen.lineTo(-width/4+(4*wx), (-2*hx)-2*ht-cx);
    pen.fill();
    pen.closePath();
    pen.beginPath();
    lightLed(num, 4);
    pen.moveTo(-width/4, (2*hx)+2*ht+cx);
    pen.lineTo(-width/6, (2*hx)+ht+cx);
    pen.lineTo(-width/6+(2*wx), (2*hx)+ht);
    pen.lineTo(-width/4+(2*wx), (2*hx));
    pen.lineTo(-width/4, (2*hx)+2*ht+cx);
    pen.fill();
    pen.closePath();
    pen.beginPath();
    lightLed(num, 5);
    pen.moveTo(width/4, (6*hx)+2*ht+cx);
    pen.lineTo(width/6, (4*hx)+ht+cx);
    pen.lineTo(-width/6, (4*hx)+ht+cx);
    pen.lineTo(-width/4, (6*hx)+2*ht+cx);
    pen.lineTo(width/4, (6*hx)+2*ht+cx);
    pen.fill();
    pen.closePath();
    pen.beginPath();
    lightLed(num, 6);
    pen.moveTo(width/4, (2*hx));
    pen.lineTo(width/6+(2*wx), (2*hx)+ht);
    pen.lineTo(width/6, (2*hx)+ht+cx);
    pen.lineTo(width/4, (2*hx)+2*ht+cx);
    pen.lineTo(width/4+(2*wx), (2*hx));
    pen.fill();
    pen.closePath();


     pen.restore();    
    
  }.bind(this);

  this.render = function(){
    this.canvas.width = this.canvas.width;
    this.digitalOutputCommonOperations();
    pen.fillStyle=this.bgcolor;
    pen.fillRect(0, 0, this.width, this.height);
    pen.lineWidth=1*this.width/60;
    pen.strokeStyle="gray";
    pen.strokeRect(0, 0, this.width, this.height);
    var o = this.splitComma();
    var s = o["string"];
    comma = o["comma"]; 
    negative = o["negative"];
    if(negative){
      pen.strokeStyle="#333";
      pen.beginPath();
      pen.moveTo(10*this.width/150, this.height/2);
      pen.lineTo(20*this.width/150, this.height/2);
      pen.closePath();
      pen.stroke();  
    }
    for(var i=0; i<s.length; i++){
      writeCipher(s[i], 2*i+1);
    }
  }
}

function LevelMeter(element){

  LevelMeter.inherits(AnalogOutput);
  AnalogOutput.call(this, element);
  
  //Object used for drawing
  var pen = this.pen;
  this.color = element.getAttribute("color") || "blue";
  this.align=element.getAttribute("align") || "vertical";
  this.topdown = element.getAttribute("topdown")=="true" || false;
  this.rounded = element.getAttribute("rounded")=="true" || false;
  this.divisions = element.getAttribute("divisions") || 25.00;
  var precision = this.precision;

  if(this.align=="vertical"){
    if(this.width==null)
      this.setWidth(100);
    if(this.height==null)
      this.setHeight(220);
  }
  else{
    if(this.width==null)
      this.setWidth(220);
    if(this.height==null)
      this.setHeight(100);
  }
  this.wv= this.width/100;
  this.hh= this.height/100;
  this.wh = this.width/220;
  this.hv = this.height/220;
  this.logfactv = Math.log(Math.E*this.wv);
  this.logfacth = Math.log(Math.E*this.hh);

  var createGradients = function(){
    if(this.align=="vertical"){
      this.grd = pen.createLinearGradient(-25*this.wv, 0, 25*this.wv, 0);
      this.grd.addColorStop(0, 'white');
      this.grd.addColorStop(1, this.color);
      this.grd2 = pen.createLinearGradient(-25*this.wv, 0, 25*this.wv, 0);
      this.grd2.addColorStop(0, 'white');
      this.grd2.addColorStop(1, '#bbb');
      this.main_grd = pen.createLinearGradient(-this.width/2, -50*this.hv, this.width/2, this.height);
      this.main_grd.addColorStop(0, 'white');
      this.main_grd.addColorStop(1, 'silver');
    }
    else{        
      this.grdo = pen.createLinearGradient(0, -25*this.hh, 0, 25*this.hh);
      this.grdo.addColorStop(0, 'white');
      this.grdo.addColorStop(1, this.color);
      this.grd2o = pen.createLinearGradient(0, -25*this.hh, 0, 25*this.hh);
      this.grd2o.addColorStop(0, 'white');
      this.grd2o.addColorStop(1, '#bbb');
      this.main_grd = pen.createLinearGradient(-this.width/2, -50*this.hh, this.width/2, this.height);
      this.main_grd.addColorStop(0, 'white');
      this.main_grd.addColorStop(1, 'silver');
    }
  }.bind(this);
  createGradients();

  this.drawLevel = function(){
    pen.save();
    pen.beginPath();
    pen.shadowColor = "#555";
    pen.strokeStyle='#555';
    if(this.align=="vertical"){
      pen.fillStyle=this.grd2;
      pen.shadowOffsetX = 1*this.wv;
      pen.shadowOffsetY = 0;
      pen.shadowBlur = 5*this.logfactv;
      pen.lineWidth=2*this.logfactv;
      if(this.rounded){
        pen.roundRect(-10*this.wv, -this.height/2+10*this.hv, 20*this.wv, this.height-20*this.hv, 10);
      }
      else{
        pen.rect(-10*this.wv, -this.height/2+10*this.hv, 20*this.wv, this.height-20*this.hv);
      }
    }
    else{
      pen.fillStyle=this.grd2o;
      pen.lineWidth=2*this.logfacth;
      pen.shadowBlur = 5*this.logfacth;
      pen.shadowOffsetX = 0;
      pen.shadowOffsetY = 1*this.hh;
      if(this.rounded){
        pen.roundRect(-this.width/2+10*this.wh, -10*this.hh, this.width-20*this.wh, 20*this.hh, 10);
      }
      else{
        pen.rect(-this.width/2+10*this.wh, -10*this.hh, this.width-20*this.wh, 20*this.hh);
      }
    }
    pen.closePath();
    pen.stroke();
    pen.fill();
    pen.restore();
  }

  var y2, x2;
  this.drawGrid=function(){ 
    var max_length=-1;
    precision = this.precision;
    for(var i=0; i<=this.divisions; i++){
      var num = Math.round((this.min_range+(this.max_range-this.min_range)*i/this.divisions)*precision)/precision; 
      pen.beginPath();
      pen.fillStyle="#000";
      var len = this.max_range.toString().length>=this.min_range.toString().length?this.max_range.toString().length:this.min_range.toString().length;
      if(precision>10)
        len = precision.toString().length+1;
      var font_ratio = len>3?len:3;
      var dx = this.divisions>50?50/this.divisions:1;
      if(this.align=="vertical"){
        pen.font='normal'+' '+'bolder'+' '+(10*this.logfactv)*3/font_ratio*dx+'px'+' '+this.fontFamily;
        y2 = (this.height/2-10*this.hv)-(this.height-20*this.hv)*i/this.divisions;
        if(this.topdown)
          y2 = -y2;
        pen.moveTo(15*this.wv, y2);
        if(i%5==0 || this.divisions/5<1){
          pen.save();
          pen.lineWidth=2*this.hv;
          pen.lineTo(25*this.wv, y2);
          pen.fillText(num, 28*this.wv, y2+(5*this.logfactv)*3/font_ratio*dx);
          pen.restore();
        }
        else{
          pen.lineTo(20*this.wv, y2);
        }
      }
      else{
        pen.font='normal'+' '+'bolder'+' '+(10*this.logfacth)*3/font_ratio*dx+'px'+' '+this.fontFamily;
        x2 = (-this.width/2+10*this.wh)+(this.width-20*this.wh)*i/this.divisions;
        if(this.topdown)
          x2 = -x2;
        pen.moveTo(x2, 15*this.hh);
        if(i%5==0 || this.divisions/5<1){
          pen.save();
          pen.lineWidth=2*this.wh;
          pen.lineTo(x2, 25*this.hh);
          pen.fillText(num, x2-pen.measureText(num).width/2, 28*this.hh+(10*this.hh)*3/font_ratio*dx);
          pen.restore();
        }
        else{
          pen.lineTo(x2, 20*this.hh);
        }
      }  
      pen.closePath();
      pen.stroke();
    }
  }
  var canv = document.createElement('canvas');
  canv.width = this.width;
  canv.height = this.height;
  var ctx = canv.getContext('2d');

  this.fillLevel = function(){
    var mheight = this.height-20*this.hv;
    var mwidth = this.width-20*this.wh;
    if(this.align=="vertical"){
      ctx.fillStyle=this.grd;
      var y;
      if(this.topdown)
        y = -mheight/2-5*this.hv;
      else
        y = mheight/2-((this.actual_value-this.min_range)*(mheight))/(this.max_range-this.min_range);
  
      ctx.fillRect(-10*this.wv, y, 20*this.wv, (((this.actual_value-this.min_range)*(mheight))/(this.max_range-this.min_range))+5*this.hv);
    }else{
      ctx.fillStyle=this.grdo;
      var x;
      if(this.topdown)
        x = mwidth/2-((this.actual_value-this.min_range)*(mwidth))/(this.max_range-this.min_range);
      else
        x = -mwidth/2-5*this.wh;
      ctx.fillRect(x, -10*this.hh, (((this.actual_value-this.min_range)*(mwidth))/(this.max_range-this.min_range))+5*this.wh, 20*this.hh);
    }
    ctx.restore();
    pen.clip();
     pen.drawImage(canv, -this.width/2, -this.height/2);
  }

  this.__defineSetter__("width", function(width){
    this._width = width;
    this.canvas.width=width;
    this.container.style.width = width+'px';
    this.wv= width/100;
    this.wh = width/220;
    this.logfactv = Math.log(Math.E*this.wv);
    this.logfacth = Math.log(Math.E*this.hh);
    if(this.height && this.width)
      createGradients();
    canv.width = width;
  });
  
  this.__defineGetter__("width", function(){
    return this._width;
   });

  this.__defineSetter__("height", function(height){
    this._height = height;
    this.canvas.height=height;
    this.container.style.height = height+'px';
    this.hh= height/100;
    this.hv = height/220;
    this.logfactv = Math.log(Math.E*this.wv);
    this.logfacth = Math.log(Math.E*this.hh);
    if(this.height && this.width)
      createGradients();
    canv.height = height;
  });
  
  this.__defineGetter__("height", function(){
    return this._height;
   });

  this.__defineSetter__("align", function(align){
    this._align = align;
    if((align=="vertical" && this.width>this.height) || (align=="horizontal" && this.height>this.width)){
      var w = this.width;
      this.width = this.height;
      this.height = w;  
      if(this.height && this.width)
        createGradients();
    }
  });

  this.__defineGetter__("align", function(){
    return this._align;
   });

  this.__defineSetter__("color", function(color){
    this._color = color;
    if(this.height && this.width)
      createGradients();
  });

  this.__defineGetter__("color", function(){
    return this._color;
   });

  this.align=element.getAttribute("align") || "vertical";
  this.color = element.getAttribute("color") || "blue";
  this.width = this.canvas.width;
  this.height = this.canvas.height;

  this.render=function(){
    this.canvas.width=this.canvas.width;
    this.analogOutputCommonOperations();
    pen.save();
    pen.translate(this.width/2, this.height/2);
    if(!this.hide_grid)
      this.drawGrid();
    this.drawLevel();
    canv.width = canv.width;
    ctx.save();
    ctx.translate(this.width/2, this.height/2);
    this.fillLevel();
    pen.restore();

  }
}

function ThermoMeter(element){

  ThermoMeter.inherits(AnalogOutput);
  AnalogOutput.call(this, element);

  var pen = this.pen;
  this.align=element.getAttribute("align") || "vertical";
  if(this.align=="vertical"){
    if(this.width==null)
      this.setWidth(100);
    if(this.height==null)
      this.setHeight(220);
  }
  else{
    if(this.width==null)
      this.setWidth(220);
    if(this.height==null)
      this.setHeight(100);
  }
  this.wv= this.width/100;
  this.hh= this.height/100;
  this.wh = this.width/220;
  this.hv = this.height/220;
  this.logfactv = Math.log(Math.E*this.wv);
  this.logfacth = Math.log(Math.E*this.hh);
  this.color = element.getAttribute("color") || "blue";
  this.divisions= element.getAttribute("divisions") || 25.00;

  var precision = this.precision;
  if(this.align=="vertical")
    var radius = Math.round(17.5*this.logfactv);
  else
    var radius = Math.round(17.5*this.logfacth);
  var grd, grd3, grd2, main_grd;
  var createGradients = function(){
    if(this.align=="vertical"){
      grd = pen.createLinearGradient(-25*this.wv, 0, 25*this.wv, 0);
      grd.addColorStop(0, 'white');
      grd.addColorStop(1, this.color);
      grd2 = pen.createLinearGradient(-25*this.wv, 0, 25*this.wv, 0);
      grd2.addColorStop(0, 'white');
      grd2.addColorStop(1, '#bbb');
      main_grd = pen.createLinearGradient(-this.width/2, -50*this.hv, this.width/2, this.height);
      main_grd.addColorStop(0, 'white');
      main_grd.addColorStop(1, 'silver');
    }
    else{        
      grdo = pen.createLinearGradient(0, -25*this.hh, 0, 25*this.hh);
      grdo.addColorStop(0, 'white');
      grdo.addColorStop(1, this.color);
      grd2o = pen.createLinearGradient(0, -25*this.hh, 0, 25*this.hh);
      grd2o.addColorStop(0, 'white');
      grd2o.addColorStop(1, '#bbb');
      main_grd = pen.createLinearGradient(-this.width/2, -50*this.hh, this.width/2, this.height);
      main_grd.addColorStop(0, 'white');
      main_grd.addColorStop(1, 'silver');
    }
  }.bind(this);
  createGradients();
  //x coord of arc's starting point
  var x1 = 0;     
  //y coord of arc's starting point  
  var y1 = 0;     
  //x coord of arc's ending point
  var x2 = 0;
  //y coord of number's starting point
  var y2 = 0;
  //y coord of first number's starting point
  var y3 = 0;
  //meter's height
  var mheight = 0;
  //meter's width
  var mwidth = 0;
  
  this.drawThermo = function(){
    pen.save();
    pen.beginPath();
    pen.shadowColor = "#555";
    pen.strokeStyle='#555';  
    if(this.align=="vertical"){
      pen.fillStyle=grd;
      pen.shadowOffsetX = 1*this.wv;
      pen.shadowOffsetY = 0*this.hv;
      pen.shadowBlur = 5*this.logfactv;
      pen.lineWidth=2*this.logfactv;
      pen.arc(0, this.height/2-10*this.hv-radius, radius, (Math.PI/180)*240, (Math.PI/180)*-60, true);
      pen.stroke();
      pen.fill();
      pen.closePath();
      pen.beginPath();
      x1 = (radius)*Math.cos(120*Math.PI/180);
      y1 = this.height/2-10*this.hv-radius-(radius)*Math.sin(120*Math.PI/180);
      x2 = (radius)*Math.cos(60*Math.PI/180)
      pen.moveTo(x1, y1);
      pen.lineTo(x1, -this.height/2+10*this.hv);
      pen.lineTo(x2, -this.height/2+10*this.hv);
      pen.lineTo(x2, y1);
      pen.fillStyle=grd2;
    }
    else{
      pen.fillStyle=grdo;
      pen.shadowOffsetX = 1*this.wh;
      pen.shadowOffsetY = 0*this.hh;
      pen.shadowBlur = 5*this.logfacth;
      pen.lineWidth=2*this.logfacth;
      pen.arc(-this.width/2+10*this.wh+radius, 0, radius, (Math.PI/180)*330, (Math.PI/180)*30, true);
      pen.stroke();
      pen.fill();
      pen.closePath();
      pen.beginPath();
      y1 = (radius)*Math.sin(330*Math.PI/180);
      x1 = -this.width/2+10*this.wh+radius+radius*Math.cos((Math.PI/180)*330);
      y2 = (radius)*Math.sin(30*Math.PI/180);
      pen.moveTo(x1, y1);
      pen.lineTo(this.width/2-10*this.wh, y1);
      pen.lineTo(this.width/2-10*this.wh, y2);
      pen.lineTo(x1, y2);
      pen.fillStyle=grd2o;
    }
    pen.stroke();
    pen.closePath();
    pen.fill();
      

    pen.shadowOffsetX = 0;
    pen.shadowOffsetY = 0;
    pen.shadowBlur = 0;
    if(this.align=="vertical"){
      pen.fillStyle=grd;
      pen.fillRect(x1, y_grid - 2*this.hv, x2-x1, radius);
      
    }
    else{
      pen.fillStyle=grdo;
      pen.fillRect(x_grid-radius, y1, radius+2*this.wh, y2-y1);
    }
  }
  var x_grid, y_grid;
  this.drawGrid = function(){
    precision = this.precision;
    for(var i=0; i<=this.divisions; i++){
      var num = Math.round((this.min_range+(this.max_range-this.min_range)*i/this.divisions)*precision)/precision; 
      pen.beginPath();
      pen.fillStyle="#000";
      var len = this.max_range.toString().length>=this.min_range.toString().length?this.max_range.toString().length:this.min_range.toString().length;
      if(precision>10)
        len = precision.toString().length+1;
      var font_ratio = len>3?len:3;
      var dx = this.divisions>50?50/this.divisions:1;  
      if(this.align=="vertical"){
        pen.font='normal'+' '+'bolder'+' '+(10*this.logfactv)*3/font_ratio*dx+'px'+' '+this.fontFamily;
        y_grid = this.height/2-10*this.hv-2*radius; 
        y2 = y_grid-(this.height-2*radius-20*this.hv)*i/this.divisions;
        pen.moveTo(15*this.wv, y2);
        if(i%5==0 || this.divisions/5<1){
          pen.save();
          pen.lineWidth=2*this.hv;
          pen.lineTo(25*this.wv, y2);
          pen.fillText(num, 28*this.wv, y2+(5*this.logfactv)*3/font_ratio);
          pen.restore();
        }
        else{
          pen.lineTo(20*this.wv, y2);
        }
      }
      else{
        pen.font='normal'+' '+'bolder'+' '+(10*this.logfacth)*3/font_ratio+'px'+' '+this.fontFamily;
        x_grid = -this.width/2+10*this.wh+2*radius;
        x2 = x_grid+(this.width-2*radius-20*this.wh)*i/this.divisions;
        pen.moveTo(x2, 15*this.hh);
        if(i%5==0 || this.divisions/5<1){
          pen.save();
          pen.lineWidth=2*this.wh;
          pen.lineTo(x2, 25*this.hh);
          pen.fillText(num, x2-pen.measureText(num).width/2, 28*this.hh+(10*this.hh)*3/font_ratio);
          pen.restore();
        }
        else{
          pen.lineTo(x2, 20*this.hh);
        }
      }  
      pen.closePath();
      pen.stroke();
    }

  }
  
  var canv = document.createElement('canvas');
  canv.width = this.width;
  canv.height = this.height;
  var ctx = canv.getContext('2d');
  
  this.fillThermo = function(){
    var mheight = this.height-20*this.hv-2*radius;
    var mwidth = this.width-20*this.wh-2*radius;
    ctx.save();
    if(this.align=="vertical"){
      ctx.translate(0, y_grid-mheight/2);
      ctx.fillStyle=grd;
      var y;
      y = mheight/2-((this.actual_value-this.min_range)*(mheight))/(this.max_range-this.min_range);
  
      ctx.fillRect(-10*this.wv, y, 20*this.wv, (((this.actual_value-this.min_range)*(mheight))/(this.max_range-this.min_range)));
    }else{
      ctx.translate(x_grid+mwidth/2, 0);
      ctx.fillStyle=grdo;
      var x;
      x = -mwidth/2;
      ctx.fillRect(x, -10*this.hh, (((this.actual_value-this.min_range)*(mwidth))/(this.max_range-this.min_range)), 20*this.hh);
    }
    ctx.restore();
    pen.clip();
     pen.drawImage(canv, -this.width/2, -this.height/2);
  }

  this.__defineSetter__("width", function(width){
    this._width = width;
    this.canvas.width=width;
    this.container.style.width = width+'px';
    this.wv= width/100;
    this.wh = width/220;
    this.logfactv = Math.log(Math.E*this.wv);
    this.logfacth = Math.log(Math.E*this.hh);
    if(this.align=="vertical")
      radius = Math.round(17.5*this.logfactv);
    else
      radius = Math.round(17.5*this.logfacth);
    if(this.height && this.width)
      createGradients();
    canv.width = width;
  });
  
  this.__defineGetter__("width", function(){
    return this._width;
  });

  this.__defineSetter__("height", function(height){
    this._height = height;
    this.canvas.height=height;
    this.container.style.height = height+'px';
    this.hh= height/100;
    this.hv = height/220;
    this.logfactv = Math.log(Math.E*this.wv);
    this.logfacth = Math.log(Math.E*this.hh);
    if(this.align=="vertical")
      radius = Math.round(17.5*this.logfactv);
    else
      radius = Math.round(17.5*this.logfacth);
    if(this.height && this.width)
      createGradients();
    canv.height = height;
  });
  
  this.__defineGetter__("height", function(){
    return this._height;
   });

  this.__defineSetter__("align", function(align){
    this._align = align;
    if((align=="vertical" && this.width>this.height) || (align=="horizontal" && this.height>this.width)){
      var w = this.width;
      this.width = this.height;
      this.height = w;
      if(align=="vertical")
        radius = Math.round(17.5*this.logfactv);
      else
        radius = Math.round(17.5*this.logfacth);
  
      if(this.height && this.width)
        createGradients();
    }
  });

  this.__defineGetter__("align", function(){
    return this._align;
  });

  this.__defineSetter__("color", function(color){
    this._color = color;
    if(this.height && this.width)
      createGradients();
  });

  this.__defineGetter__("color", function(){
    return this._color;
   });
  this.align=element.getAttribute("align") || "vertical";
  this.color = element.getAttribute("color") || "blue";
  this.width = this.canvas.width;
  this.height = this.canvas.height;
  this.render = function(){
    this.canvas.width=this.canvas.width;
    this.analogOutputCommonOperations();
    pen.save();
    pen.translate(this.width/2, this.height/2);
    if(!this.hide_grid)
      this.drawGrid();
    this.drawThermo();
    canv.width = canv.width;
    ctx.save();
    ctx.translate(this.width/2, this.height/2);
    this.fillThermo();
    pen.restore();
  }

}

function Graph(element){
  Graph.inherits(AnalogOutput);
  AnalogOutput.call(this, element);
  if(this.width==null)
    this.setWidth(450);
  if(this.height==null)
    this.setHeight(300);
  var canvas = new Array(3);
  canvas[0] = this.canvas;
  canvas[0].setAttribute("id", "first");
  canvas[0].style.display="block";
  canvas[1] = document.createElement("canvas");
  canvas[1].setAttribute("style", canvas[0].getAttribute("style"));
  canvas[1].setAttribute("id", "second");
  canvas[1].width = canvas[0].width;
  canvas[1].height = canvas[0].height;
  canvas[1].style.display="none";
  canvas[2] = document.createElement("canvas");
  canvas[2].setAttribute("style", canvas[0].getAttribute("style"));
  canvas[2].setAttribute("id", "third");
  canvas[2].width = canvas[0].width;
  canvas[2].height = canvas[0].height;
  canvas[2].style.display="block";
  canvas[2].style.zIndex="3";
  canvas[2].style.position="absolute";
  
  this.container.appendChild(canvas[1]);
  this.container.appendChild(canvas[2]);
  var pen = new Array(3);
  pen[0] = this.pen;
  pen[1] = canvas[1].getContext('2d');
  pen[2] = canvas[2].getContext('2d');
  this.wx = this.width/460;
  this.hx = this.width/300;
  var cindex=0;    
  var on = 0;
  var standby = false;
  var start = 40*this.wx;
  var starty = 10*this.hx;
  var c=0;
  var v=false;
  this.channel = new Array();
  this.divisions = element.getAttribute("hide_divisions") || 30;
  this.hide_divisions = element.getAttribute("hide_divisions")=="true" || false;
  this.hide_axis = element.getAttribute("hide_axis")=="true" || false;
  this.scale = element.getAttribute("scale") || "range";
  this.bgcolor = element.getAttribute("bgcolor") || "black";
  var power_on_load = element.getAttribute("power_onload")=="true" || false;
  this.setMode = function(m){
    switch(m){
      case "ch1":
        mode = 0;
        break;
      case "ch2":
        mode = 1;
        break;
      case "dual":
        mode = 2;
        break;
      case "x/y":
        mode = 3;
        break;
    }
    this.clearScreen();
    canvas[1-cindex].style.display="none";
    canvas[cindex].style.display="block";
  }

  this.on = function(){
    on = 1;
  }
  
  this.off = function(){
    on = 0;
    k=0;
    for(var i in this.channel){
      this.channel[i].reset();
        this.channel[i].end = false;
        this.channel[i].index=0;
    }  
    canvas[0].width = canvas[0].width;
    canvas[0].style.backgroundColor = '#000';
    canvas[1].width = canvas[1].width;
    canvas[1].style.backgroundColor = '#000';
    canvas[1-cindex].style.display="none";
    canvas[cindex].style.display="block";
  }

  this.isOn = function(){
    return on==1;
  }

  this.isOff = function(){
    return on==0;
  }


  this.clearScreen = function(){
    canvas[cindex].width = canvas[cindex].width;
    canvas[cindex].style.backgroundColor = this.bgcolor;
  }
  function Channel(id, obs, item){
    this.__defineSetter__("amplitude", function(amp){
      this._amplitude = amp;
      v=true;
      obs.clearScreen();
    });

    this.__defineGetter__("amplitude", function(){
      return this._amplitude;
     });

    this.__defineSetter__("amp4div", function(amp){
      this._amp4div = amp;
      if(obs.scale=="divisions")
        this._amplitude = -(1/(8*amp));
      v=true;
      obs.clearScreen();
    });

    this.__defineGetter__("amp4div", function(){
      return this._amp4div;
     });

    this.__defineSetter__("sweep", function(s){
      this._sweep = (1/s)*((this.divfactor));
      v=true;
      obs.clearScreen();
    });

    this.__defineGetter__("sweep", function(){
      return this._sweep;
     });
    this.divfactor = 1;
    this.sweep = parseFloat(item.getAttribute("sweep")) || 1;

    this.__defineSetter__("x_position", function(x){
      this._x_position = x;
      v=true;
      obs.clearScreen();
    });

    this.__defineGetter__("x_position", function(){
      return this._x_position;
     });

    this.__defineSetter__("y_position", function(y){
      this._y_position = y;
      v=true;
      obs.clearScreen();
    });

    this.__defineGetter__("y_position", function(){
      return this._y_position;
     });
    
    this.reset = function(){
      this.refresh = start;
      this.end = true;
    }

    this.getPeakToPeak = function(){
      return peak*2;
    }

    this.getPeak = function(){
      return peak;
    }
    
    this.getAverage = function(){
      return average;
    }

    this.setPoints = function(p){
      peak = null;
      for(var i in p){
        if(p[i]>peak || !peak)
          peak = p[i];
        temp_average+=parseFloat(p[i]);
        this.point[i] = p[i];
      }
      var prec = obs.precision;
      average = Math.round(temp_average/p.length*prec)/prec;
      var div = this.divfactor;
      this.divfactor = (obs.width-80*obs.wx)/(10*this.point.length);
      this.sweep = div/this.sweep;

    }

    var peak = null;
    var average = 0;
    var temp_average = 0;
    this.frequency = parseFloat(item.getAttribute("frequency")) || 1;
    this.amplitude = parseFloat(item.getAttribute("amplitude")) || 1;
    this.amp4div = parseFloat(item.getAttribute("amp4div")) || 1;
    this.index = 0;
    this.y_position = parseFloat(item.getAttribute("y_position")) || 0;
    this.x_position = parseFloat(item.getAttribute("x_position")) || 0;
    this.point = new Array();
    this.prec_value=0;
    this.end=false;
    this.refresh = start; 
    this.color = item.getAttribute("color") || "lawngreen";
    this.href = item.getAttribute("href") || null;
    this.oncompletescreen = item.getAttribute("oncompletescreen") || "";
    this.ajax = new Ajax();
  }
  var z = 0;
  for(var i=0; i<element.childNodes.length; i++){
    if(element.childNodes[i].nodeName.toLowerCase()=="channel" && z<2){
      element.childNodes[i].style.display='none';
      this.channel[z] = new Channel(z, this, element.childNodes[i]);
      if(element.childNodes[i].innerHTML!='')
        this.channel[z].setPoints(JSON.parse(element.childNodes[i].innerHTML));
      z++;
    }
  }

  var first=true;
  var time = this.refresh;
  var k=0;
  var mode=0;
  var temp_mode = element.getAttribute("mode") || "ch1";
  switch(temp_mode){
    case "ch1":
      mode = 0;
      break;
    case "ch2":
      mode = 1;
      break;
    case "dual":
      mode = 2;
      break;
    case "x/y":
      mode = 3;
      break;
  }
  var resolution=1;

  var main_grd;
  var createGradients = function(){
    main_grd = pen[cindex].createLinearGradient(-this.width, this.height/2, this.width, this.height/2);
    main_grd.addColorStop(0, '#bbb');
    main_grd.addColorStop(0.5, '#eee');
    main_grd.addColorStop(1, '#bbb');
  }.bind(this);
  createGradients();
  
  var drawGrid = function(){
    canvas[0].style.backgroundColor = this.bgcolor;
    canvas[1].style.backgroundColor = this.bgcolor;
    pen[2].strokeStyle = "black";
    pen[2].save();
    pen[2].fillStyle = main_grd;
    pen[2].fillRect(0, 0, 40*this.wx, this.height);
    pen[2].fillRect(0, 0, this.width, 10*this.hx);
    pen[2].fillRect(0, this.height-(10*this.hx), this.width, (10*this.hx));
    pen[2].fillRect(this.width-(40*this.wx), 0, 40*this.wx, this.height);
  
    pen[2].restore();

    pen[2].strokeStyle = "#555";
    pen[2].strokeRect(0, 0, this.width, this.height);
    var y2=0;
    var x2=0;
    for(var i=1; i<10; i++){
      pen[2].beginPath();
      if(i!=5 && !this.hide_divisions)
        pen[2].dashedLineTo(40*this.wx+i*(this.width-80*this.wx)/10, 10*this.hx, 40*this.wx+i*(this.width-80*this.wx)/10, this.height-10*this.hx);
      else if(i==5){
        if(!this.hide_axis){
          pen[2].moveTo(40*this.wx+i*(this.width-80*this.wx)/10, 10*this.hx);
          pen[2].lineTo(40*this.wx+i*(this.width-80*this.wx)/10, this.height-10*this.hx);
        }
      }
      pen[2].stroke();
      pen[2].closePath();
      pen[2].beginPath();
    }
    for(var i=1; i<8; i++){
      if(i!=4 && !this.hide_divisions)
        pen[2].dashedLineTo(40*this.wx, 10*this.hx+i*(this.height-20*this.hx)/8, this.width-40*this.wx, 10*this.hx+i*(this.height-20*this.hx)/8);
      else if(i==4){
        if(!this.hide_axis){
          pen[2].moveTo(40*this.wx, 10*this.hx+i*(this.height-20*this.hx)/8);
          pen[2].lineTo(this.width-40*this.wx, 10*this.hx+i*(this.height-20*this.hx)/8);
        }
      }
      pen[2].stroke();
      pen[2].closePath();
    }
    if(!this.hide_axis){    
      for(var i=1; i<40; i++){
        pen[2].beginPath();
        y2 = (this.height-(10*this.hx))-(this.height-(20*this.hx))*i/40;
        pen[2].moveTo(this.width/2-3*this.wx, y2);
        pen[2].lineTo(this.width/2+3*this.wx, y2);
        pen[2].closePath();
        pen[2].stroke();
        pen[2].beginPath();
        x2 = (this.width-(40*this.wx))-(this.width-(80*this.wx))*i/40;
        pen[2].moveTo(x2, this.height/2-3*this.hx);
        pen[2].lineTo(x2, this.height/2+3*this.hx);
        pen[2].closePath();
        pen[2].stroke();
      }
    }
    if(!this.hide_grid){
      var maxx = this.max_range.toString().length;
      var minn = this.min_range.toString().length;
      var font = ((maxx>5 || minn>5)?(8-((maxx>minn?maxx:minn)-4))*this.wx:8*this.wx);
      pen[2].font='normal'+' '+'bolder'+' '+font+'px'+' '+this.fontFamily;
      var precision = this.precision;
      for(var i=0; i<=this.divisions; i++){
        pen[2].beginPath();
        y2 = (this.height-(10*this.hx))-(this.height-(20*this.hx))*i/this.divisions;
        pen[2].moveTo(35*this.wx, y2);
        num = Math.round((this.min_range+(this.max_range-this.min_range)*i/this.divisions)*precision)/precision; 
        if(i%5==0){
          pen[2].save();
          pen[2].fillStyle="#333";
          pen[2].lineTo(26*this.wx, y2);
          pen[2].fillText(num, 25*this.wx-5*(num.toString().length>4?4:num.toString().length), y2+font/2);
          pen[2].restore();
  
        }
        else{
          pen[2].lineTo(30*this.wx, y2);
        }
        pen[2].closePath();
    
        pen[2].stroke();
      }
    }  
  }.bind(this);  


  
  var drawWaveXY = function(){
    if(on==0)
      return;
    pen[cindex].strokeStyle = this.channel[0].color;
    this.prec_value=this.channel[0].point[this.channel[0].index];
    this.value = this.channel[1].point[this.channel[1].index];
    if(k>420*this.wx){
      eval(this.channel[0].oncompletescreen);
      this.channel[0].reset();
      eval(this.channel[1].oncompletescreen);
      this.channel[1].reset();
    }
    var hvalue, xvalue;
    if(this.scale=="range"){
      hvalue =starty+this.channel[1].y_position+((this.max_range - (this.channel[1].point[this.channel[1].index]*this.channel[1].amplitude))*(this.height-(20*this.hx)))/(this.max_range-this.min_range);
      xvalue= start+this.channel[0].x_position+((this.max_range - (this.channel[0].point[this.channel[0].index]*this.channel[0].amplitude))*(this.height-(20*this.hx)))/(this.max_range-this.min_range);
    }
    else if(this.scale=="divisions"){
      hvalue =starty+(this.height-20*this.hx)/2+this.channel[1].y_position+(((this.channel[1].point[this.channel[1].index]*this.channel[1].amplitude))*(this.height-(20*this.hx)));
      xvalue= start+(this.width-80*this.wx)/2+this.channel[0].x_position+(((this.channel[0].point[this.channel[0].index]*this.channel[0].amplitude))*(this.height-(20*this.hx)));
    }      

    if(hvalue > (12*this.hx) && hvalue < (this.height-(23*this.hx))){
      pen[cindex].beginPath();
      pen[cindex].arc(xvalue, hvalue, 1, 0*Math.PI/180, 360 * Math.PI/180, false);
      pen[cindex].closePath();
      pen[cindex].stroke();
      pen[cindex].beginPath();
      pen[cindex].save();
      pen[cindex].lineWidth=2;
      if(this.scale=="range")
        pen[cindex].moveTo(start+this.channel[0].x_position+((this.max_range - (this.channel[0].point[this.channel[0].index-1]*this.channel[0].amplitude))*(this.height-(20*this.hx)))/(this.max_range-this.min_range), starty+this.channel[1].y_position+((this.max_range - (this.channel[1].prec_value*this.channel[1].amplitude))*(this.height-(20*this.hx)))/(this.max_range-this.min_range));
      else if(this.scale=="divisions")
        pen[cindex].moveTo(start+(this.width-80*this.wx)/2+this.channel[0].x_position+(((this.channel[0].point[this.channel[0].index-1]*this.channel[0].amplitude))*(this.height-(20*this.hx))), starty+(this.height-20*this.hx)/2+this.channel[1].y_position+(((this.channel[1].prec_value*this.channel[1].amplitude))*(this.height-(20*this.hx))));

      pen[cindex].lineTo(xvalue, hvalue);        
      pen[cindex].stroke();
      pen[cindex].closePath();
      pen[cindex].restore();
      this.channel[0].prec_value=this.channel[0].point[this.channel[0].index];
      this.channel[1].prec_value=this.channel[1].point[this.channel[1].index];
    }

    if(this.channel[0].point.length && this.channel[1].point.length){  
      this.channel[0].index=(this.channel[0].index+resolution)%this.channel[0].point.length;
      this.channel[1].index=(this.channel[1].index+resolution)%this.channel[1].point.length;
    }
    
  }.bind(this);
  var test="";
  var once = false;
  var drawWave = function(ch){
    if(on==0 || this.channel[ch].point[this.channel[ch].index]==undefined)
      return;
    this.prec_value=this.value;
    this.value = this.channel[ch].point[this.channel[ch].index];
    pen[cindex].strokeStyle = this.channel[ch].color;
    this.channel[ch].refresh = this.channel[ch].x_position+start+(this.channel[ch].sweep/this.channel[ch].frequency)*k;
    if(this.channel[ch].refresh>=420*this.wx){
      once = true;
      eval(this.channel[ch].oncompletescreen);
      this.channel[ch].reset();
    }
    var hvalue; 
    if(this.scale=="range")
      hvalue =starty+this.channel[ch].y_position+((this.max_range - (this.channel[ch].point[this.channel[ch].index]*this.channel[ch].amplitude))*(this.height-20*this.hx))/(this.max_range-this.min_range);
    else if(this.scale=="divisions")
      hvalue =starty+(this.height-20*this.hx)/2+this.channel[ch].y_position+(((this.channel[ch].point[this.channel[ch].index]*this.channel[ch].amplitude))*(this.height-20*this.hx));
    if(hvalue > (12*this.hx) && hvalue < (this.height-(12*this.hx)) && this.channel[ch].refresh>start && this.channel[ch].refresh<410*this.wx){
      pen[cindex].beginPath();
      pen[cindex].arc(this.channel[ch].refresh, hvalue, 1, 0*Math.PI/180, 360 * Math.PI/180, false);
      pen[cindex].closePath();
      pen[cindex].stroke();
      pen[cindex].beginPath();
      pen[cindex].save();
      pen[cindex].lineWidth=2;

      if((this.channel[ch].index-resolution)>0){
        if(this.scale=="range")
            pen[cindex].moveTo(this.channel[ch].x_position+start+((this.channel[ch].sweep/this.channel[ch].frequency)*(k-resolution)), starty+this.channel[ch].y_position+((this.max_range - (this.channel[ch].point[this.channel[ch].index-resolution]*this.channel[ch].amplitude))*(this.height-(20*this.hx)))/(this.max_range-this.min_range));
        else if(this.scale=="divisions")
            pen[cindex].moveTo(this.channel[ch].x_position+start+((this.channel[ch].sweep/this.channel[ch].frequency)*(k-resolution)), starty+(this.height-20*this.hx)/2+this.channel[ch].y_position+(((this.channel[ch].point[this.channel[ch].index-resolution]*this.channel[ch].amplitude))*(this.height-(20*this.hx))));
        pen[cindex].lineTo(this.channel[ch].x_position+start+(this.channel[ch].sweep/this.channel[ch].frequency)*k, hvalue);        
      }          
      pen[cindex].stroke();
      pen[cindex].closePath();
      pen[cindex].restore();
    }
    if(this.channel[ch].point.length)
      this.channel[ch].index=(this.channel[ch].index+resolution)%this.channel[ch].point.length;
  }.bind(this);

  this.__defineSetter__("max_range", function(max_r){
    this._max_range = max_r;
    if(!first)
      drawGrid();
  });

  this.__defineGetter__("max_range", function(){
    return this._max_range;
  });

  this.max_range = parseFloat(element.getAttribute("max_range")) || 5;

  this.__defineSetter__("min_range", function(min_r){
    this._min_range = min_r;
    if(!first)
      drawGrid();
  });

  this.__defineGetter__("min_range", function(){
    return this._min_range;
  });

  this.min_range = parseFloat(element.getAttribute("min_range")) || -5;
  this.value=this.prec_value=this.min_range || 0;

  this.__defineSetter__("width", function(width){
    this._width = width;
    canvas[0].width=width;
    this.container.style.width = width+'px';
    this.wx = width/460;
    start = 40*this.wx;
    canvas[1].width = width;    
    canvas[2].width = width;
    createGradients();    
    drawGrid();
    for(var i in this.channel){
      var div = this.channel[i].divfactor;
      if(this.channel[i].point.length)
        this.channel[i].divfactor = (this.width-80*this.wx)/(10*this.channel[i].point.length);
      this.channel[i].sweep = div/this.channel[i].sweep;
    }

  });

  this.__defineGetter__("width", function(){
    return this._width;
  });
  
  this.width = this.canvas.width;

  this.__defineSetter__("height", function(height){
    this._height = height;
    canvas[0].height=height;
    this.container.style.height = height+'px';
    this.hx = height/300;
    starty = 10*this.hx;
    canvas[1].height = height;
    canvas[2].height = height;
    createGradients();    
    drawGrid();
  });

  this.__defineGetter__("height", function(){
    return this._height;
  });

  this.height = this.canvas.height;


  var draw = function(){
    var check=true;
    this.analogOutputCommonOperations();
    switch(mode){
      case 0:
        drawWave(0);
        check = this.channel[0].end;      
        break;
      case 1:
        drawWave(1);
        check = this.channel[1].end;      
        break;
      case 2:
        for(var i=0; i<this.channel.length; i++){
          drawWave(i);
          check = check && this.channel[i].end;
        }
        break;
      case 3:
        drawWaveXY();
        check = check && this.channel[0].end && this.channel[1].end;
        break;
    }
    if(check){
      k=0;
      cindex = 1 - cindex;
      v=false;
      this.clearScreen();
      canvas[cindex].style.display="none";
      canvas[1-cindex].style.display="block";
      for(var i=0; i<this.channel.length; i++){
        this.channel[i].end = false;
        this.channel[i].index=0;
      }
    }
    else
      k+=(resolution);
    
    if(on==1)
      setTimeout(draw, 10);    

  }.bind(this);

  
  this.render = function(){
    if(power_on_load)
      this.on();
    if(first==false && time>=this.refresh){
      if(this.channel[0] && this.channel[0].href!=null){
          this.channel[0].ajax.addCall("if(ajaxobj.responseText != '') "+element.id+".channel[0].setPoints(JSON.parse(ajaxobj.responseText));");
          this.channel[0].ajax.load(this.channel[0].href, null);
      }
      if(this.channel[1] && this.channel[1].href!=null){
          this.channel[1].ajax.addCall("if(ajaxobj.responseText != '') "+element.id+".channel[1].setPoints(JSON.parse(ajaxobj.responseText));");
          this.channel[1].ajax.load(this.channel[1].href, null);
      }
      time=0;
    }    
    if(!first && on){
      time+=0.1;
    }  
    if(on==1){
      first=false;
      draw();
    }

  
  }
}

function GaugeAdv(element){
  GaugeAdv.inherits(Instrument);
  Instrument.call(this, element);
  var radius = element.getAttribute("radius") || "";
  var rx = radius/100;
  this.setWidth(radius*2+36*rx);
  this.setHeight(radius*2+36*rx);

  var style = element.getAttribute("style") || ""; 
  var led_color = element.getAttribute("led_color") || ""; 
  var max_range = element.getAttribute("max_range") || ""; 
  var min_range = element.getAttribute("min_range") || ""; 
  var range_from = element.getAttribute("range_from") || "";
  var range_to = element.getAttribute("range_to") || "";
  var parent_id = element.getAttribute("id") || "";
  var cipher = element.getAttribute("cipher") || "";
  var significative = element.getAttribute("significative") || "";
  var value = element.getAttribute("value") || "";
  var onalert = element.getAttribute("onalert") || "this.inner[1].on();";
  var onleavealert = element.getAttribute("onleavealert") || "this.inner[1].off();";
  
  var gau = Drinks.createElement("display", {"id": parent_id+"gau", "min_range":min_range, "max_range":max_range, "range_from":range_from, "range_to":range_to, "style":style, "value":value, "radius":radius});
  this.appendChild(gau);

  var disp = Drinks.createElement("display", {"id": parent_id+"dis", "type":"digital", "width":40*rx, "height":20*rx, "x":100*rx, "y":160*rx, "link":"true", "cipher": cipher, "significative":significative, "min_range":min_range, "max_range":max_range,});
  this.inner[0].appendChild(disp);
  
  var ledd = Drinks.createElement("led", {"id":parent_id+"led", "radius":5*rx, "x":112*rx, "y":190*rx, "color":led_color});
  this.inner[0].appendChild(ledd);
  
  this.inner[0].onalert=onalert;
  this.inner[0].onleavealert=onleavealert;

  this.render = function(){
    this.instrumentCommonOperations();
  }
}
