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

function Ajax(){
  var ajaxobj = new XMLHttpRequest();
  var loader = null;
  var callee = null;

  this.setLoader = function(src){
    loader = src;  
  }

  var get = function(url){
    ajaxobj.open("get", url, true);
    ajaxobj.send(null);
  }

  var post = function(url, string){
    ajaxobj.open("post", url, true);
    ajaxobj.setRequestHeader("content-type", "application/x-www-form-urlencoded");
    ajaxobj.send(string);
  }

  this.addCall=function(f){
    calls[calls.length] = f;
  }

  var response = function(id){
    if(id != null)
      document.getElementById(id).innerHTML=loader;
    var self = this;
    ajaxobj.onreadystatechange=function()
    {
      if(ajaxobj.readyState==4 && ajaxobj.status==200){
        if(id != null)
          document.getElementById(id).innerHTML=ajaxobj.responseText;
        for(var c in calls){
          eval(calls[c]);
        }
        calls = new Array();        
      }
    }
  }.bind(this);

  this.load = function(file, loc){
    get(file);
    response(loc);
  }

  this.send = function(form, url, loc){
    var string="";
    for(var i = 0; i < form.elements.length; i++){
      if(i < form.elements.length-1){
        string += form.elements[i].name+"="+encodeURIComponent(form.elements[i].value)+"&";
      }else{
        string += form.elements[i].name+"="+encodeURIComponent(form.elements[i].value);
      }
    }
    post(url, string);
    response(loc);
  }
}

//var ajax = new Ajax();

if (!Function.prototype.bind) {
  Function.prototype.bind = function(){ 
    var fn = this, args = Array.prototype.slice.call(arguments),
        object = args.shift(); 
    return function(){ 
      return fn.apply(object, 
        args.concat(Array.prototype.slice.call(arguments))); 
    }; 
  };
}

CanvasRenderingContext2D.prototype.dashedLineTo = function (fromX, fromY, toX, toY) {
  var lt = function (a, b) { return a <= b; };
  var gt = function (a, b) { return a >= b; };
  var capmin = function (a, b) { return Math.min(a, b); };
  var capmax = function (a, b) { return Math.max(a, b); };
  var checkX = { thereYet: gt, cap: capmin };
  var checkY = { thereYet: gt, cap: capmin };
  if (fromY - toY > 0) {
    checkY.thereYet = lt;
    checkY.cap = capmax;
  }
  if (fromX - toX > 0) {
    checkX.thereYet = lt;
    checkX.cap = capmax;
  }
  this.moveTo(fromX, fromY);
  var offsetX = fromX;
  var offsetY = fromY;
  var idx = 0, dash = true;
  while (!(checkX.thereYet(offsetX, toX) && checkY.thereYet(offsetY, toY))) {
    var ang = Math.atan2(toY - fromY, toX - fromX);
    var len = 5;
    offsetX = checkX.cap(toX, offsetX + (Math.cos(ang) * len));
    offsetY = checkY.cap(toY, offsetY + (Math.sin(ang) * len));
    if (dash) this.lineTo(offsetX, offsetY);
    else this.moveTo(offsetX, offsetY);
    idx = (idx + 1) % 5;
    dash = !dash;
  }
};

CanvasRenderingContext2D.prototype.roundRect = function (fromx, fromy, width, height, offset) {
  this.save();
  this.translate(fromx, fromy);
  this.beginPath();
  this.moveTo(offset, 0);
  this.lineTo(width-offset, 0);
  this.quadraticCurveTo(width, 0, width, offset);
  this.lineTo(width, height-offset);
  this.quadraticCurveTo(width, height, width-offset, height);
  this.lineTo(offset, height);    
  this.quadraticCurveTo(0, height, 0, height-offset);
  this.lineTo(0, offset);    
  this.quadraticCurveTo(0, 0, offset, 0);
  this.closePath();
  this.restore();
}

function getElementsByAttribute(attr, value){
  var arrElements = document.getElementsByTagName("*");
  var arrReturnElements = new Array();
  for(var i=0; i<arrElements.length; i++){
    if(arrElements[i].getAttribute(attr)==value)
      arrReturnElements.push(arrElements[i]);
  }
  return arrReturnElements;
}
var all_type={"knob":{"loop":"LoopKnob", "analog":"EndedKnob", "digital":"DigitalKnob"}, "display":{"gauge":"Gauge", "digital": "dMeter", "level": "LevelMeter", "thermo":"ThermoMeter", "analog":"VuMeter", "graph":"Graph"}, "led":{"round":"RoundLed", "rect":"RectLed", "triangle":"TriangleLed"}, "switch":{"rocker":"RockerSwitch", "arc":"ArcSwitch", "side":"SideSwitch", "circle": "CircleSwitch", "rect":"RectSwitch", "toggle":"ToggleSwitch"}, "oscope":"Oscilloscope", "slider":{"analog":"AnalogSlider", "digital":"DigitalSlider"}, "arcled":"ArcLed", "gaugeadv":"GaugeAdv"};
var default_type={"knob":"loop", "display":"gauge", "led":"round", "switch":"toggle", "slider":"analog"};
//window.addEventListener('load', initialize, false);
var earr = [];



function debug(id, msg){
 document.getElementById(id).innerHTML = msg;
}

function Drinks(){};

Drinks.buildElement = function(element){
  var tag = element.nodeName;
  var temp, type;
  if(all_type[tag.toLowerCase()]){
    if(have_parent(element) && !earr[element.id]){
      if(typeof(all_type[tag.toLowerCase()])=="object"){
        temp = all_type[tag.toLowerCase()];
        type = temp[element.getAttribute("type") || default_type[tag.toLowerCase()]];
        if(element.getAttribute("type")==null)
          element.setAttribute("type", default_type[tag.toLowerCase()]);
      }
      else
        type = all_type[tag.toLowerCase()];
      eval('earr[element.id] = new '+type+'(element)');
      eval(element.id+'=earr[element.id];');
      eval('if(Drinks.'+tag.toLowerCase()+'s){Drinks.'+tag.toLowerCase()+'s.push(earr[element.id]);}else{Drinks.'+tag.toLowerCase()+'s=new Array(); Drinks.'+tag.toLowerCase()+'s.push(earr[element.id]);}');
    }
  }
}

Drinks.appendChild = function(id, element){
  var parent = document.getElementById(id);
  parent.appendChild(element);
  Drinks.buildElement(element);
}

Drinks.ready = function(){};

function initialize(){
  var arr = document.getElementsByTagName("*");
  var type;
  var temp = {}
  for(var i=0; i<arr.length; i++){
    Drinks.buildElement(arr.item(i));
  }
  Drinks.ready.call();
/*
  function render(){
    for(var e in earr)
      earr[e].render();      
  }
  setInterval(render, 100);  
*/
}

Drinks.getElementById = function(id){
  return earr[id];
}

Drinks.getElementsByName = function(n){
  var arr = new Array();
  for(var i in earr){
    if(earr[i].name==n)
      arr.push(earr[i]);
  }
  return arr;
}

Drinks.getElementsByClassName = function(c){
  var arr = new Array();
  for(var i in earr){
    if(earr[i].className==c)
      arr.push(earr[i]);
  }
  return arr;
}

function have_parent(el){
  return all_type[el.parentNode.nodeName.toLowerCase()]==null;
}


function attach(element, type, fn){
  if (element.addEventListener){
    element.addEventListener(type, fn, false);
  } else if (element.attachEvent){
    element.attachEvent('on' + type, fn);
  }
}

function detach(element, type, fn){
  if (element.removeEventListener){
    element.removeEventListener(type, fn, false);
  } else if (element.attachEvent){
    element.detachEvent('on' + type, fn);
  }
}

function getAbsolutePosition(element) {
  var r = { x: element.offsetLeft, y: element.offsetTop };
  if (element.offsetParent) {
      var tmp = getAbsolutePosition(element.offsetParent);
      r.x += tmp.x;
      r.y += tmp.y;
  }
  return r;
};

function getRelativeCoordinates(event, reference) {
  var x, y;
  event = event || window.event;
  var el = event.target || event.srcElement;

  if (!window.opera && typeof event.offsetX != 'undefined') {
     var pos = { x: event.offsetX, y: event.offsetY };
      var e = el;
      while (e) {
        e.mouseX = pos.x;
        e.mouseY = pos.y;
        pos.x += e.offsetLeft;
        pos.y += e.offsetTop;
        e = e.offsetParent;
      }
      var e = reference;
      var offset = { x: 0, y: 0 }
     while (e) {
        if (typeof e.mouseX != 'undefined') {
            x = e.mouseX - offset.x;
            y = e.mouseY - offset.y;
            break;
        }
      offset.x += e.offsetLeft;
      offset.y += e.offsetTop;
      e = e.offsetParent;
      }

      e = el;
      while (e) {
        e.mouseX = undefined;
        e.mouseY = undefined;
        e = e.offsetParent;
      }  
  }
  else {
      var pos = getAbsolutePosition(reference);
      x = event.pageX  - pos.x;
      y = event.pageY - pos.y;
  }
  return { x: x, y: y };
}


Drinks.createElement = function(tag, obj){
  var element = document.createElement(tag);
  for(var i in obj){
    element.setAttribute(i, obj[i]);
  }
  return element;
}

Drinks.createManager = function(){
  if(!Drinks.managers)
    Drinks.managers = new Array();
  var man = new Manager();  
  Drinks.managers.push(man);
  return man;
}

function Instrument(element){
  this.value = element.getAttribute("value")!=null?parseFloat(element.getAttribute("value")):(parseFloat(element.getAttribute("min_range")) || 0);
  this.canvas = document.createElement('canvas');
  this.label_container = document.createElement('div');
  this.pen = this.canvas.getContext('2d');
  this.min_range=element.hasAttribute("min_range")?parseFloat(element.getAttribute("min_range")) : 0;
  this.max_range=element.hasAttribute("max_range")?parseFloat(element.getAttribute("max_range")) : 100;
  this.name=element.getAttribute("name") || null;
  this.className=element.getAttribute("class") || null;
  this.fontFamily = element.getAttribute("font") || "sans";
  this.ajax = null;
  this.inner = new Array();
  this.href = element.getAttribute("href") || null;
  this.onchange=element.getAttribute("onchange") || null;
  this.onload=element.getAttribute("onload") || null;
  var first_time = true;
  this.label = element.getAttribute("label") || null;
  this.rotate = element.getAttribute("rotate") || null;
  this.prec_value = this.value;
  this.refresh = parseInt(element.getAttribute("refresh")) || 5;
  this.type = element.getAttribute("type") || null;
  var refresh_time = 0;
  if(element.getAttribute("name")){
    this.hidden = document.createElement('input');
    this.hidden.setAttribute("type", "hidden");
    this.hidden.setAttribute("name", element.getAttribute("name"));
    element.appendChild(this.hidden);
    element.removeAttribute("name");
  }
    
  if(element.getAttribute("radius")){
    this.width=parseInt(element.getAttribute("radius"))*2;
    this.height = parseInt(element.getAttribute("radius"))*2;
  }
  else{  
    this.width=parseInt(element.getAttribute("width")) || null;
    this.height=parseInt(element.getAttribute("height")) || null;
  }
  this.canvas.width=this.width;
  this.canvas.height=this.height;
  this.parent = element.parent || null;
  this.container = document.createElement('div');
  var rotstyle="";
  if(this.rotate)
    rotstyle = "transform: rotate("+this.rotate+"deg); -ms-transform: rotate("+this.rotate+"deg); -webkit-transform: rotate("+this.rotate+"deg); -o-transform: rotate("+this.rotate+"deg); -moz-transform: rotate("+this.rotate+"deg);";  
  var szstyle = "";
  if (this.width)
    szstyle += " width:"+this.width+"px;";
  if (this.height)
    szstyle += " height:"+this.height+"px;"
  this.container.setAttribute("style", "position:relative;"+szstyle+rotstyle);
  if(this.parent){
    this.container.setAttribute("style", "position:absolute;"+szstyle+rotstyle);
    this.canvas.setAttribute("style", "position: absolute;");
    this.container.appendChild(this.canvas);
    this.parent.container.appendChild(this.container);
  }else{
    this.canvas.setAttribute("style", "float:left;");
    this.container.appendChild(this.canvas);
    element.appendChild(this.container);
    if(this.label){
      var label = document.createElement("div");
      label.setAttribute("style", "float:left; min-height:20px; width:100%; padding-top:2px; background-color:white; border:1px solid silver; text-align:center;");
      label.innerHTML=element.getAttribute("label");
      this.container.style.height = (this.height+parseInt(label.style.offsetHeight))+'px';
      this.container.appendChild(label);
    }  
  }

  this.html = element;
  this.x=0;
  this.y=0;
  this.ajax = new Ajax();
         

  this.__defineSetter__("id", function(id){
    var old_id = this._id;
    this._id = id;    
    eval("earr[id]= this;");
    eval(id+"= this;");
  });

  this.__defineGetter__("id", function(){
    return this._id;
  });
  this.id=element.getAttribute("id") || null;

  this.__defineSetter__("rotate", function(rotate){
    this._rotate = rotate;
    var rotstyle="";
    if(this.rotate)
      rotstyle = "transform: rotate("+this.rotate+"deg); -ms-transform: rotate("+this.rotate+"deg); -webkit-transform: rotate("+this.rotate+"deg); -o-transform: rotate("+this.rotate+"deg); -moz-transform: rotate("+this.rotate+"deg);";  
    this.container.setAttribute("style", this.container.getAttribute("style")+rotstyle);
  });
  
  this.__defineGetter__("rotate", function(){
    return this._rotate;
   });

  this.appendChild = function(el){
    var el_name = el.nodeName.toLowerCase();
    if(all_type[el_name]!=null){
      var temp, temp_obj, type;
      el.parent = this;
      if(typeof(all_type[el_name])=="object"){
        temp_obj = all_type[el_name];
        type = temp_obj[el.getAttribute("type") || default_type[el_name]];
        if(el.getAttribute("type")==null)
          el.setAttribute("type", default_type[el_name]);
        }
      else
        type = all_type[el_name];
      eval("temp = new "+type+"(el);");
      temp.x = parseFloat(el.getAttribute("x")) || 0;    
      temp.y = parseFloat(el.getAttribute("y")) || 0;
      temp.canvas.style.top = '0px';
      temp.canvas.style.left = '0px';
      temp.container.style.top = temp.y+'px';
      temp.container.style.left = temp.x+'px';
      this.inner.push(temp);  
      eval(el.id+" = temp;");
    }  
  }

  for(var i=0; i<element.childNodes.length; i++){
    var el = element.childNodes[i];
    this.appendChild(el);
  }

  var renderInner = function(){
    var value = this.actual_value!=undefined?this.actual_value:this.value;
    for(var i in this.inner){
      this.pen.save();
      this.pen.translate(this.inner[i].x, this.inner[i].y);
      if(this.inner[i].html.getAttribute("link")!=null)
        this.inner[i].value=value;
      this.inner[i].render();
      this.pen.restore();
    }
  }.bind(this);

  this.instrumentCommonOperations = function(){
    if(first_time){
      eval(this.onload);
      first_time = false;
    }
    if(this.href){
      if(refresh_time>=this.refresh){
        if(this.output){
          this.ajax.addCall("if(ajaxobj.responseText!='') "+this.id+".value=ajaxobj.responseText;");
          this.ajax.load(this.href, null);
        }
        else if(this.input){
          this.ajax.load(this.href+"?value="+this.value, null);
        }
        refresh_time=0;    
      }else{
        refresh_time+=0.1;
      }
    }
    if(this.displays && this.input){  
      for(var d in this.displays){
        if(this.displays[d].id)
          eval(this.displays[d].id+'.value=this.value;');  
      }    
    }
    if(this.prec_value!=this.value){
      eval(this.onchange);
      this.prec_value = this.value;
    }
    if(this.hidden)
      this.hidden.value=this.value;
    renderInner();

  }

  this.setWidth = function(w){
    this.canvas.width=w;
    this.width = w;
    this.container.style.width = this.width+'px';
  }

  this.setHeight = function(h){
    this.canvas.height=h;
    this.height = h;
    if(label)
      this.container.style.height = (this.height+parseInt(label.style.offsetHeight))+'px';
    else
      this.container.style.height = this.height+'px';
  }

}

function Manager(){
  this.href="";
  this.refresh=5;
  var id = Drinks.managers.length || 0;
  this.values = {};
  this.input = null;
  var ajax = new Ajax();

  var run = function(){
    if(this.input){
      var string = this.href+'?';
      for(var i in this.input){
        if(i < this.input.length-1){
                string += this.input[i]+"="+encodeURIComponent(Drinks.getElementById(this.input[i]).value)+"&";
            }else{
                string += this.input[i]+"="+encodeURIComponent(Drinks.getElementById(this.input[i]).value);
            }
      }      
      ajax.load(string, null);
    }else{
      if(this.href && this.href!=""){
        ajax.addCall("if(ajaxobj.responseText!=''){Drinks.managers["+id+"].values=JSON.parse(ajaxobj.responseText);}");
        ajax.load(this.href, null);
        for(var i in this.values){
          earr[i].value = this.values[i];
        }
      }
    }
  }.bind(this);

  this.start = function(){
    setInterval(run, this.refresh*1000);
  }

}

function Analog(element){
  this.hide_grid = element.getAttribute("hide_grid")=="true" || false;
  this.divisions=100;

  this.__defineSetter__("precision", function(precision){
    if(precision=="auto")
      this._precision = precision;
    else  
      this._precision = Math.pow(10, precision);
  });  

  this.__defineGetter__("precision", function(){
    if(this._precision=="auto"){
      var how = (this.max_range-this.min_range).toString();
      var index = how.indexOf(".");
      var cipher;
      if(index>0)
        cipher = how.length-index-1;
      else if((this.min_range)/this.divisions<=1/10)
        cipher = 1;
      else
        cipher = -1
      this._precision = Math.pow(10, cipher);
    }
    return this._precision;
  });
  this.precision=element.getAttribute("precision") || "auto";

  this.analogCommonOperations = function(){
    
  }
}

function Digital(element){

  this.digitalCommonOperations = function(){
    
  }

}

function Input(element){
  Input.inherits(Instrument);
  Instrument.call(this, element);
  this.displays = element.getAttribute('display')!=null?getElementsByAttribute("display_name", element.getAttribute('display')):null;
  this.input=true;

  this.inputCommonOperations = function(){
    this.instrumentCommonOperations();
  };

}

function AnalogInput(element){
  AnalogInput.inherits(Input);
  Input.call(this, element);
  AnalogInput.inherits(Analog);
  Analog.call(this, element);

  this.analogInputCommonOperations = function(){
    this.inputCommonOperations();
    this.analogCommonOperations();
  }
}

function Binary(element){
  this.min_range=0;
  this.max_range=1;

  this.on = function(){
    this.value=1;
  }

  this.off = function(){
    this.value=0;
  }

  this.toggle = function(){
    this.value^=1;
  }

  this.binaryCommonOperations = function(){

  }
}

function BinaryInput(element){
  BinaryInput.inherits(Input);
  Input.call(this, element);
  BinaryInput.inherits(Binary);
  Binary.call(this, element);

  this.binaryInputCommonOperations = function(){
    this.inputCommonOperations();
    this.binaryCommonOperations();
  }
}

function DigitalInput(element){
  DigitalInput.inherits(Input);
  Input.call(this, element);
  DigitalInput.inherits(Digital);
  Digital.call(this, element);

  this.digitalInputCommonOperations = function(){
    this.inputCommonOperations();
    this.digitalCommonOperations();
  }

}

function MoveInput(element){
  var drag=0;
  var move=null;
  this.x_root=0;
  this.y_root=0;
  this.up_func=null;
  this.move_func=null;
  this.click_func=null;
  this.decx=0;
  this.decy=0;

  this.canvas.style.cursor='pointer';
  this.canvas.addEventListener("mousedown", function(event){if(event.button==0)clickHandler(event);}, false);
  this.canvas.addEventListener("touchstart", function(event){clickHandler(event);}, false);
  this.canvas.addEventListener("mouseup", function(event){mouseUp(event);}, false);
  this.canvas.addEventListener("touchend", function(event){mouseUp(event);}, false);
  this.canvas.addEventListener("selectstart", function(){return false;}, false);
  this.canvas.addEventListener("dragstart", function(){return false;}, false);
  this.canvas.addEventListener("drag", function(){return false;}, false);
  this.canvas.addEventListener("mouseout", function(){
    this.canvas.style.cursor='pointer';
    if(this.up_func && drag==1)
      this.up_func();
    drag=0;
    this.canvas.removeEventListener("mousemove",move, false);
    this.canvas.removeEventListener("touchmove", move, false);
  }.bind(this), false);

  var mouseUp = function(event){  
    if(drag){
      var obj = getRelativeCoordinates(event, this.canvas);
      this.x_root = obj.x-this.decx;
      this.y_root = obj.y-this.decy;
      this.canvas.style.cursor='pointer';
      if(this.up_func)
        this.up_func();
      drag=0;
      this.canvas.removeEventListener("mousemove",move, false);
      this.canvas.removeEventListener("touchmove", move, false);
    }
  }.bind(this);

  var moveHandler = function(event){
    var obj = getRelativeCoordinates(event, this.canvas);
    this.canvas.style.cursor='move';
    this.x_root = obj.x-this.decx;
    this.y_root = obj.y-this.decy;
    if(this.move_func)
      this.move_func();
  }.bind(this);

  var clickHandler = function(event){
    var obj = getRelativeCoordinates(event, this.canvas);
    move = function(event){moveHandler(event);};
    this.canvas.addEventListener("mousemove", move, false);
    this.canvas.addEventListener("touchmove", move, false);
    this.canvas.focus();
    drag=1;
    this.x_root = obj.x-this.decx;
    this.y_root = obj.y-this.decy;
    if(this.click_func)
      this.click_func();
  }.bind(this);

  this.moveInputCommonOperations = function(){
  }

}

function Output(element){
  Output.inherits(Instrument);
  Instrument.call(this, element);
  this.output=true;

  this.outputCommonOperations=function(){
    this.instrumentCommonOperations();
  }

}

function An2DigOutput(element){
  An2DigOutput.inherits(Output);
  Output.call(this, element);

  this.autoscale=(element.getAttribute("autoscale")=="true") || false;
  this.range_from = parseFloat(element.getAttribute('range_from'))==null?null:element.getAttribute('range_from');
  this.range_to = parseFloat(element.getAttribute('range_to'))==null?null:element.getAttribute('range_to');
  this.onalert = element.getAttribute("onalert") || null;
  this.onleavealert = element.getAttribute("onleavealert") || null;
   var once_alert = false;

  this.an2digoutputCommonOperations = function(){
    this.outputCommonOperations();
    
    //Autoscale
    if (this.autoscale)
    {
      if (this.value>this.max_range){
        this.max_range = this.max_range*2;
      }
      if(this.value<this.min_range && this.autoscale){
        if(this.min_range>=0)
          this.min_range = -this.max_range;
        else 
          this.min_range = this.min_range*2;
      }      
    }
    
    //Limitting
    var beg=Math.min(this.min_range, this.max_range);
    var end=Math.max(this.min_range, this.max_range);
    if (this.value>end)
      this.value=end;
    if(this.value<beg)
      this.value=beg;

    //Start OnAlert
    var val = this.value;
    if(this.actual_value!=undefined)
      val = this.actual_value;

    if(this.range_from!=null && this.range_to!=null){
      if(val>=this.range_from && val<=this.range_to && !once_alert){  
        eval(this.onalert);
        once_alert = true;    
      }
      else if(val<this.range_from || val>this.range_to){
        //Start OnLeaveAlert
        if(once_alert)
          eval(this.onleavealert);
        //End OnLeaveAlert
        once_alert = false;
      }
    }
    //End OnAlert
  }

}

function AnalogOutput(element){
  AnalogOutput.inherits(An2DigOutput);
  An2DigOutput.call(this, element);
  AnalogOutput.inherits(Analog);
  Analog.call(this, element);
  this.position=0;
  this.prec_position=0;
  this.actual_value = this.value;
  var time=0;
  var b=0;
  this.text=element.getAttribute("text") || "";
  
  this.analogOutputCommonOperations=function(){

    //Start Hysteresis
    this.position = (parseFloat(this.value)-this.min_range)*this.divisions/(this.max_range-this.min_range);
    this.actual_value=Math.round((((this.prec_position/this.divisions)*(this.max_range-this.min_range))+this.min_range)*10)/10;

    if((this.prec_position>=(this.position+0.2) || this.prec_position<=(this.position-0.2))){
      time+=0.1;
      if(this.prec_position<this.position){
        if(b){
          time/=2;
        }
        b=0;
        this.prec_position+=9.81*Math.pow(time,2)/2;
      }
      else if(this.prec_position>this.position){
        if(!b){
          time/=2;
        }
        b=1;
        this.prec_position-=9.81*Math.pow(time,2)/2;
      }  
      this.prec_position = this.prec_position<0?0:this.prec_position;
      this.prec_position = this.prec_position>this.divisions?this.divisions:this.prec_position;
      
    }
    else{
      time=0;
      this.prec_position = this.position;    
    }
    //End Hysteresis
    this.an2digoutputCommonOperations();
    this.analogCommonOperations();
  }

}

function BinaryOutput(element){
  BinaryOutput.inherits(Output);
  Output.call(this, element);
  BinaryOutput.inherits(Binary);
  Binary.call(this, element);

  this.binaryOutputCommonOperations=function(){
    this.outputCommonOperations();
    this.binaryCommonOperations();
  }
}

function DigitalOutput(element){
  DigitalOutput.inherits(An2DigOutput);
  An2DigOutput.call(this, element);
  DigitalOutput.inherits(Digital);
  Digital.call(this, element);
  
  this.cipher=parseInt(element.getAttribute('cipher')) || 2;
  this.significative = parseInt(element.getAttribute("significative")) || null;
  
  this.digitalOutputCommonOperations = function(){
    this.an2digoutputCommonOperations();
    this.digitalCommonOperations();
  }

  this.splitComma = function(){
    var comma=0;
    var negative=false;    
    if(this.significative==null)
      this.value = parseInt(this.value);
    if(this.value<0)
      negative = true;  
    if(this.significative)
      this.value = Math.round(this.value*10*this.significative)/(10*this.significative);
    var s = this.value?Math.abs(this.value).toString():"0";
    var b=0;
    if(s.indexOf('.')+1==s.length){
      b=1;      
      for(var i=0; i<this.significative; i++){
        s+='0';
      }
      for(var i=s.length-1; i<this.cipher; i++){
        s='0'+s;
      }
    }

    if(this.significative && this.value%1==0 && s.indexOf('.')==-1){
      if(s.length<this.cipher)
        s+='.';
      for(var i=0; i<this.significative; i++){
        s+='0';
      }
      for(var i=s.length-1; i<this.cipher; i++){
        s='0'+s;
      }
    }
    else if(this.significative && b!=1){
      for(var i=(s.length-1-s.indexOf('.')); i<this.significative; i++){
        s+='0';
      }
      for(var i=s.length-1; i<this.cipher; i++){
        s='0'+s;
      }
    }
      
    
    if(s.length<this.cipher){
      for(var i=0; i<this.cipher; i++){
        if(i>=s.length)    
          s = '0'+s;  
      }
    }
    if(s.indexOf('.')!=-1){
      comma = s.indexOf('.');  

      s = s.replace('.', '');
    }
    else
      comma = null;
    return {"string":s, "comma":comma, "negative":negative};
  }
}

Function.prototype.inherits = function(superclass) {
  var temp = function() {};
  temp.prototype = superclass.prototype;
  this.prototype = new temp();
}

function lerp(a, b, fac) {
  var ret = [];

  if(a == b) {
      return a;
  }

  for (var i = 0; i < Math.min(a.length, b.length); i++) {
      ret[i] = a[i] * (1 - fac) + b[i] * fac;
  }

  return ret;
}

function project(target, initial, current) {
  var delta = initial.sub(target);

  if( (delta.x == 0) && (delta.y == 0) ) {
      return target;
  }

  var t = current.sub(target).mul(delta).div(delta.toDistSquared());

  return delta.mul(t.x + t.y).add(target);
}

function dot(a, b) {
  return a.x * b.x + a.y * b.y;
}

function Circle(location, radius) {
  this.init(location, radius);
}

Circle.prototype = {
  init: function(location, radius) {
      this.location = location? new Point(location[0], location[1]): new Point();
      this.radius = radius? radius: 1;
  },
  contains: function(p) {
      return p.sub(this.location).toDist() <= this.radius;
  },
  intersect: function(p1, p2) {
    // http://local.wasp.uwa.edu.au/~pbourke/geometry/sphereline/
    var dp = p2.sub(p1);
    var a = dp.toDistSquared();
    var b = 2 * (dp.x * (p1.x - this.location.x) +
        dp.y * (p1.y - this.location.y));
    var c = this.location.toDistSquared() + p1.toDistSquared() - 2 *
        (this.location.x * p1.x + this.location.y * p1.y) -
        this.radius * this.radius;

    var bb4ac = b * b - 4 * a * c;

    var epsilon = 0.0001;
    if (Math.abs(a) < epsilon || bb4ac < 0) {
        return [];
    }

    if (bb4ac == 0) {
        return [p2.sub(p1).mul(-b / (2 * a)).add(p1)];
    }

    var mu1 = (-b + Math.sqrt(bb4ac)) / (2 * a);
    var mu2 = (-b - Math.sqrt(bb4ac)) / (2 * a);

    return [p2.sub(p1).mul(mu1).add(p1), p2.sub(p1).mul(mu2).add(p1)]
  }
}

function Point(x, y) {
  this.init(x, y);
}

Point.prototype = {
  init: function(x, y) {
      this.x = x? x: 0;
      this.y = y? y: 0;
  },
  add: function(other) {
    return this._operationTemplate(other, function(a, b) {return a + b});
  },
  sub: function(other) {
    return this._operationTemplate(other, function(a, b) {return a - b});
  },
  mul: function(other) {
    return this._operationTemplate(other, function(a, b) {return a * b});
  },
  div: function(other) {
    return this._operationTemplate(other, function(a, b) {return a / b});
  },
  ceil: function() {
    return this._operationTemplate(null, function(a) {return Math.ceil(a)});
  },
  floor: function() {
    return this._operationTemplate(null, function(a) {return Math.floor(a)});
  },
  round: function() {
      return this._operationTemplate(null, function(a) {return Math.round(a)});
  },
  _operationTemplate: function(other, op) {
    if(isNumber(other)) {
      return new Point(op(this.x, other), op(this.y, other));
    }

    if(other == null) {
      return new Point(op(this.x), op(this.y));
    }

    return new Point(op(this.x, other.x), op(this.y, other.y));
  },
  toDist: function() {
      return Math.sqrt(this.toDistSquared());
  },
  toDistSquared: function() {
      return this.x * this.x + this.y * this.y;
  },
  normalize: function() {
      return this.div(this.toDist());
  },
  invert: function() {
      return new Point(-this.x, -this.y);
  },
  closest: function(points) {
    return this._findTemplate(points,
      function() {
        return Number.MAX_VALUE;
      },
      function(dist, recordDist) {
        return dist < recordDist;
      }
    );
  },
  farthest: function(points) {
    return this._findTemplate(points,
      function() {
        return 0;
      },
      function(dist, recordDist) {
        return dist > recordDist;
      }
    );
  },
  _findTemplate: function(points, init, compare) {
    var record = init();
    var recordPoint = points[0];

    for (var i = 1; i < points.length; i++) {
      var point = points[i];
      var dist = this.sub(point).toDist();

      if (compare(dist, record)) {
        record = dist;
        recordPoint = point;
      }
    }

    return recordPoint;
  }
}

function isNumber(n) {
  return !isNaN(parseFloat(n)) && isFinite(n);
}

function process(canvas, func, pen, x_root) {
  function setPixel(imageData, x, y, rgba) {
    var index = (x + y * imageData.width) * 4;

    for (var i = 0; i < 4; i++) {
      imageData.data[index + i] = rgba[i] * 255;
    }
  }

  var imageData = pen.createImageData(canvas.width,
      canvas.height);

  for (var y = 0; y < canvas.height; y++) {
    for (var x = 0; x < canvas.width; x++) {
      var result = func(new Point(x, y), canvas, x_root);
      setPixel(imageData, x, y, result);
    }
  }
  var c = document.createElement('canvas');
  c.width = canvas.width;
  c.height = canvas.height;
  var ctx = c.getContext('2d');
  ctx.putImageData(imageData, 0, 0);
  pen.drawImage(c, 0, 0);
}


var colors = [ 
  [0.85, 0.85, 0.85, 1], // red
  [0.95, 0.95, 0.95, 1],
  [0.85, 0.85, 0.85, 1], // blue
  [0.8, 0.8, 0.8, 1], // blue
  [0.85, 0.85, 0.85, 1], // red
  [0.95, 0.95, 0.95, 1],
  [0.85, 0.85, 0.85, 1], // blue
  [0.8, 0.8, 0.8, 1], // blue
  [0.85, 0.85, 0.85, 1], // red
  [0.95, 0.95, 0.95, 1],
  [0.85, 0.85, 0.85, 1], // blue
  [0.8, 0.8, 0.8, 1], // blue
  [0.85, 0.85, 0.85, 1], // red
  [0.95, 0.95, 0.95, 1],
  [0.85, 0.85, 0.85, 1], // blue
  [0.8, 0.8, 0.8, 1], // blue
];

var angularGradient = function(point, canvas, x_root) {
  // figure out angle
  var gradientCenter=new Point(x_root, canvas.height/2);
  var dir = point.sub(gradientCenter);
  var angle = Math.atan2(dir.y, dir.x);

  // wrap around as positive
  if (dir.y < 0 ) {
    angle += 2 * Math.PI;
  }

  // map to [0, 1] range
  angle /= (2 *Math.PI);

  // figure out which segments to interpolate
  var angleRatio = angle * colors.length;
  var index = Math.floor(angleRatio);
  var leftColor = index == 0? colors[colors.length - 1]:
    colors[index - 1];

  var rightColor = colors[index];

  // figure out interpolation factor
  var lerpFac = angleRatio % 1;
  return lerp(leftColor, rightColor, lerpFac);
}

