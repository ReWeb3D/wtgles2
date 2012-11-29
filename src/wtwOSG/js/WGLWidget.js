/*
 * Copyright (C) 2010 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

/* Note: this is at the same time valid JavaScript and C++. */

WT_DECLARE_WT_MEMBER
(1, JavaScriptConstructor, "WGLWidget",
 function(APP, canvas) {
   jQuery.data(canvas, 'obj', this);

   var self = this;
   var WT = APP.WT;

   // TG: add view member
   var view = this;
   this._canvas = canvas;
   this._mouseWheelEventNode = canvas;
   this._mouseEventNode = canvas;
   this._keyboardEventNode = document;
   
   var vec3 = WT.glMatrix.vec3;
   var mat3 = WT.glMatrix.mat3;
   var mat4 = WT.glMatrix.mat4;

   this.ctx = null;

   // Placeholders for the initializeGL and paintGL functions,
   // which will be overwritten by whatever is rendered
   this.initializeGL = function() {};
   this.paintGL = function() {};
   this.resizeGL = function() {};
   this.updates = new Array();
   this.initialized = false;
   this.preloadingTextures = false;

   // interaction specific stuff
   // var dragPreviousXY = null;
   // var lookAtCenter = null;
   // var lookAtUpDir = null;
   // var lookAtPitchRate = 0;
   // var lookAtYawRate = 0;
   self.cameraMatrix = null;
   // var walkFrontRate = 0;
   // var walkYawRate = 0;
	var manipulator = 0;
   
   this.setManipulator = function(manipulator)
   {
		this.manipulator = manipulator;
   }
   
   this.getManipulator = function()
   {
		return this.manipulator;
   }
   
   this.discoverContext = function(noGLHandler) {
     if (canvas.getContext) {
       try {
         this.ctx = canvas.getContext('webgl', {antialias: true});
       } catch (e) {}
       if (this.ctx == null) {
         try {
           this.ctx = canvas.getContext('experimental-webgl');
         } catch (e) {}
       }
       if (this.ctx == null) {
         var alternative = canvas.firstChild;
         var parentNode = canvas.parentNode;
         parentNode.insertBefore(alternative, canvas);
         canvas.style.display = 'none';
         noGLHandler();
       }
     }
     return this.ctx;
   };
   
	this.setLookToFromUpParams = function(matrix, from, center, up) {
		this.setupManipulator();
		// TODO: set up using home position
		
		this.getManipulator().setLookFromToUp(from, center, up);
		
		// does not work, therefore use WtMatrix0 directly :-(
		self.cameraMatrix = matrix;
			
		// execute function for the first time 
		self.timeTick();
   }
   
   
   // todo: poll the matrix and call render regulary
	self.timeTick = function()
	{ 
		// make a generic requestAnimationFrame method
		if (!window.requestAnimationFrame) 
		{
			window.requestAnimationFrame = (function() 
			{return window.requestAnimationFrame || window.webkitRequestAnimationFrame ||
				window.mozRequestAnimationFrame || window.oRequestAnimationFrame ||        function(/* function FrameRequestCallback */ callback, /* DOMElement Element */ element) 
				{
					window.setTimeout(callback, 1000/60);  // timeout : 1/60 s
				};
			})();
		}

		self.getManipulator().update(new Date().getTime());
		self.cameraMatrix = self.getManipulator().getInverseMatrix();
		// use hack HACK
		self.ctx.WtMatrix0 = self.cameraMatrix;
		self.paintGL();

		// request next time tick
		window.requestAnimationFrame(self.timeTick);
	}
   
   // TG: some dummy methods setup by Wt
   this.mouseUp = function(o, event) {}
   this.mouseDown = function(o, event) {}
   this.mouseDragLookAt = function(o, event) {}
   this.mouseWheelLookAt = function(o, event) {}
   
   // TG: taken from osgViewer/Viewer.js
   this.setupManipulator = function(manipulator, dontBindDefaultEvent) {
        if (manipulator === undefined) {
            manipulator = new osgGA.OrbitManipulator();
        }

		// TG: disable this
        // if (manipulator.setNode !== undefined) {
            // manipulator.setNode(this.getSceneData());
        // } else {
            // // for backward compatibility
            // manipulator.view = this;
        // }

        this.setManipulator(manipulator);

        var that = this;
        var viewer = this;
	var fixEvent = function( event ) {

            //if ( event[ expando ] ) {
                //return event;
            //}

            // store a copy of the original event object
            // and "clone" to set read-only properties

            // nop
            //var originalEvent = event;
            //event = jQuery.Event( originalEvent );

            for ( var i = this.props.length, prop; i; ) {
                prop = this.props[ --i ];
                event[ prop ] = originalEvent[ prop ];
            }

            // Fix target property, if necessary
            if ( !event.target ) {
                event.target = event.srcElement || document; // Fixes #1925 where srcElement might not be defined either
            }

            // check if target is a textnode (safari)
            if ( event.target.nodeType === 3 ) {
                event.target = event.target.parentNode;
            }

            // Add relatedTarget, if necessary
            if ( !event.relatedTarget && event.fromElement ) {
                event.relatedTarget = event.fromElement === event.target ? event.toElement : event.fromElement;
            }

            // Calculate pageX/Y if missing and clientX/Y available
            if ( event.pageX === null && event.clientX !== null ) {
                var doc = document.documentElement, body = document.body;
                event.pageX = event.clientX + (doc && doc.scrollLeft || body && body.scrollLeft || 0) - (doc && doc.clientLeft || body && body.clientLeft || 0);
                event.pageY = event.clientY + (doc && doc.scrollTop  || body && body.scrollTop  || 0) - (doc && doc.clientTop  || body && body.clientTop  || 0);
            }

            // Add which for key events
            if ( !event.which && ((event.charCode || event.charCode === 0) ? event.charCode : event.keyCode) ) {
                event.which = event.charCode || event.keyCode;
            }

            // Add metaKey to non-Mac browsers (use ctrl for PC's and Meta for Macs)
            if ( !event.metaKey && event.ctrlKey ) {
                event.metaKey = event.ctrlKey;
            }

            // Add which for click: 1 === left; 2 === middle; 3 === right
            // Note: button is not normalized, so don't use it
            if ( !event.which && event.button !== undefined ) {
                event.which = (event.button & 1 ? 1 : ( event.button & 2 ? 3 : ( event.button & 4 ? 2 : 0 ) ));
            }

            return event;
        };

        if (dontBindDefaultEvent === undefined || dontBindDefaultEvent === false) {

            var disableMouse = false;

            var touchstart = function(ev)
            {
                //disableMouse = true;
                return viewer.getManipulator().touchstart(ev);
            };
            var touchend = function(ev)
            {
                //disableMouse = true;
                return viewer.getManipulator().touchend(ev);
            };
            var touchmove = function(ev)
            {
                //disableMouse = true;
                return viewer.getManipulator().touchmove(ev);
            };

            var touchcancel = function(ev)
            {
                //disableMouse = true;
                return viewer.getManipulator().touchcancel(ev);
            };

            var touchleave = function(ev)
            {
                //disableMouse = true;
                return viewer.getManipulator().touchleave(ev);
            };

            // iphone/ipad
            var gesturestart = function(ev)
            {
                return viewer.getManipulator().gesturestart(ev);
            };
            var gesturechange = function(ev)
            {
                return viewer.getManipulator().gesturechange(ev);
            };
            var gestureend = function(ev)
            {
                return viewer.getManipulator().gestureend(ev);
            };

            // touch events
            this._canvas.addEventListener("touchstart", touchstart, false);
            this._canvas.addEventListener("touchend", touchend, false);
            this._canvas.addEventListener("touchmove", touchmove, false);
            this._canvas.addEventListener("touchcancel", touchcancel, false);
            this._canvas.addEventListener("touchleave", touchleave, false);

            // iphone/ipad 
            this._canvas.addEventListener("gesturestart", gesturestart, false);
            this._canvas.addEventListener("gesturechange", gesturechange, false);
            this._canvas.addEventListener("gestureend", gestureend, false);

            // mouse
            var mousedown = function (ev)
            {
                if (disableMouse === false) {
                    return viewer.getManipulator().mousedown(ev);
                }
            };
            var mouseup = function (ev)
            {
                if (disableMouse === false) {
                    return viewer.getManipulator().mouseup(ev);
                }
            };
            var mousemove = function (ev)
            {
                if (disableMouse === false) {
                    return viewer.getManipulator().mousemove(ev);
                }
            };
            var dblclick = function (ev)
            {
                if (disableMouse === false) {
                    return viewer.getManipulator().dblclick(ev);
                }
            };
            var mousewheel = function (event)
            {
                if (disableMouse === false) {
                    // from jquery
                    var orgEvent = event || window.event, args = [].slice.call( arguments, 1 ), delta = 0, returnValue = true, deltaX = 0, deltaY = 0;
                    //event = $.event.fix(orgEvent);
                    event.type = "mousewheel";
                    
                    // Old school scrollwheel delta
                    if ( event.wheelDelta ) { delta = event.wheelDelta/120; }
                    if ( event.detail     ) { delta = -event.detail/3; }
                    
                    // New school multidimensional scroll (touchpads) deltas
                    deltaY = delta;
                    
                    // Gecko
                    if ( orgEvent.axis !== undefined && orgEvent.axis === orgEvent.HORIZONTAL_AXIS ) {
                        deltaY = 0;
                        deltaX = -1*delta;
                    }
                    
                    // Webkit
                    if ( orgEvent.wheelDeltaY !== undefined ) { deltaY = orgEvent.wheelDeltaY/120; }
                    if ( orgEvent.wheelDeltaX !== undefined ) { deltaX = -1*orgEvent.wheelDeltaX/120; }
                    // Add event and delta to the front of the arguments
                    args.unshift(event, delta, deltaX, deltaY);
                    var m = viewer.getManipulator();
                    return m.mousewheel.apply(m, args);
                }
            };

            if (viewer.getManipulator().mousedown) {
                this._mouseEventNode.addEventListener("mousedown", mousedown, false);
            }
            if (viewer.getManipulator().mouseup) {
                this._mouseEventNode.addEventListener("mouseup", mouseup, false);
            }
            if (viewer.getManipulator().mousemove) {
                this._mouseEventNode.addEventListener("mousemove", mousemove, false);
            }
            if (viewer.getManipulator().dblclick) {
                this._mouseEventNode.addEventListener("dblclick", dblclick, false);
            }
            if (viewer.getManipulator().mousewheel) {
                this._mouseWheelEventNode.addEventListener("DOMMouseScroll", mousewheel, false);
                this._mouseWheelEventNode.addEventListener("mousewheel", mousewheel, false);
            }

            var keydown = function(ev) {return viewer.getManipulator().keydown(ev); };
            var keyup = function(ev) {return viewer.getManipulator().keyup(ev);};

            if (viewer.getManipulator().keydown) {
                this._keyboardEventNode.addEventListener("keydown", keydown, false);
            }
            if (viewer.getManipulator().keyup) {
                this._keyboardEventNode.addEventListener("keyup", keyup, false);
            }

            var self = this;
            
			// TG: removed this resize functionality from osgJS completely
			// var resize = function(ev) {
                // var w = window.innerWidth;
                // var h = window.innerHeight;

                // var prevWidth = self._canvas.width;
                // var prevHeight = self._canvas.height;
                // self._canvas.width = w;
                // self._canvas.height = h;
                // self._canvas.style.width = w;
                // self._canvas.style.height = h;
                // osg.log("window resize "  + prevWidth + "x" + prevHeight + " to " + w + "x" + h);
                
				// // TG: implement for resize and ratio adaptation
				// // var camera = self.getCamera();
                // // var vp = camera.getViewport();
                // // var widthChangeRatio = w/vp.width();
                // // var heightChangeRatio = h/vp.height();
                // // var aspectRatioChange = widthChangeRatio / heightChangeRatio; 
                // // vp.setViewport(vp.x()*widthChangeRatio, vp.y()*heightChangeRatio, vp.width()*widthChangeRatio, vp.height()*heightChangeRatio);

                // // if (aspectRatioChange !== 1.0) {

                    // // osg.Matrix.postMult(osg.Matrix.makeScale(1.0, aspectRatioChange, 1.0 ,[]), camera.getProjectionMatrix());
                // // }
            // };
            //TG: disable window.onresize = resize;
        }
    }
     
   // // interaction specific stuff
   // this.setLookAtParams = function(matrix, center, up, pitchRate, yawRate) {
     // cameraMatrix = matrix;
     // lookAtCenter = center;
     // lookAtUpDir = up;
     // lookAtPitchRate = pitchRate;
     // lookAtYawRate = yawRate;
   // };

   // this.mouseDragLookAt = function(o, event) {
     // if (this.ctx == null) return; // no WebGL support
     // var c = WT.pageCoordinates(event);
     // var dx=(c.x - dragPreviousXY.x);
     // var dy=(c.y - dragPreviousXY.y);
     // var s=vec3.create();
     // s[0]=cameraMatrix[0];
     // s[1]=cameraMatrix[4];
     // s[2]=cameraMatrix[8];
     // var r=mat4.create();
     // mat4.identity(r);
     // mat4.translate(r, lookAtCenter);
     // mat4.rotate(r, dy * lookAtPitchRate, s);
     // mat4.rotate(r, dx * lookAtYawRate, lookAtUpDir);
     // vec3.negate(lookAtCenter);
     // mat4.translate(r, lookAtCenter);
     // vec3.negate(lookAtCenter);
     // mat4.multiply(cameraMatrix,r,cameraMatrix);
     // //console.log('mouseDragLookAt after: ' + mat4.str(cameraMatrix));
     // // Repaint!
     // //console.log('mouseDragLookAt: repaint');
     // this.paintGL();
     // // store mouse coord for next action
     // dragPreviousXY = WT.pageCoordinates(event);
   // };

   // // Mouse wheel = zoom in/out
   // this.mouseWheelLookAt = function(o, event) {
     // if (this.ctx == null) return; // no WebGL support
     // WT.cancelEvent(event);
     // //alert('foo');
     // var d = WT.wheelDelta(event);
     // var s = Math.pow(1.2, d);
	 // var test = lookAtCenter;
	 // test*=s;
     // mat4.translate(cameraMatrix, lookAtCenter);
     // mat4.scale(cameraMatrix, [s, s, s]);
     // vec3.negate(lookAtCenter);
     // mat4.translate(cameraMatrix, lookAtCenter);
     // vec3.negate(lookAtCenter);
     // // Repaint!
     // this.paintGL();
   // };

   // this.setWalkParams = function(matrix, frontRate, yawRate) {
     // cameraMatrix = matrix;
     // walkFrontRate = frontRate;
     // walkYawRate = yawRate;
   // };

   // this.mouseDragWalk = function(o, event){
     // if (this.ctx == null) return; // no WebGL support
     // var c = WT.pageCoordinates(event);
     // var dx=(c.x - dragPreviousXY.x);
     // var dy=(c.y - dragPreviousXY.y);
     // var r=mat4.create();
     // mat4.identity(r);
     // mat4.rotateY(r, dx * walkYawRate);
     // var t=vec3.create();
     // t[0]=0;
     // t[1]=0;
     // t[2]=-walkFrontRate * dy;
     // mat4.translate(r, t);
     // mat4.multiply(r, cameraMatrix, cameraMatrix);
     // this.paintGL();
     // dragPreviousXY = WT.pageCoordinates(event);
   // };


   // this.mouseDown = function(o, event) {
     // WT.capture(null);
     // WT.capture(canvas);

     // dragPreviousXY = WT.pageCoordinates(event);
   // };

   // this.mouseUp = function(o, event) {
     // if (dragPreviousXY != null)
       // dragPreviousXY = null;
   // };

 });
