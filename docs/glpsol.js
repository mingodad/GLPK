/**
 * @license
 * Copyright 2014 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

// WebGLWorker client code

function assert(x) {
  if (!x) throw 'failed assert';
}

function WebGLClient() {
  var objects = {};

  var ctx = null;
  var buffer = null;
  var i = 0;
  var skippable = false;
  var currFrameBuffer = null;

  function func0(name) {
    ctx[name]();
  }
  function func1(name) {
    ctx[name](buffer[i]);
    i++;
  }
  function func2(name) {
    ctx[name](buffer[i], buffer[i+1]);
    i += 2;
  }
  function func3(name) {
    ctx[name](buffer[i], buffer[i+1], buffer[i+2]);
    i += 3;
  }
  function func4(name) {
    ctx[name](buffer[i], buffer[i+1], buffer[i+2], buffer[i+3]);
    i += 4;
  }
  function func5(name) {
    ctx[name](buffer[i], buffer[i+1], buffer[i+2], buffer[i+3], buffer[i+4]);
    i += 5;
  }
  function func6(name) {
    ctx[name](buffer[i], buffer[i+1], buffer[i+2], buffer[i+3], buffer[i+4], buffer[i+5]);
    i += 6;
  }
  function func7(name) {
    ctx[name](buffer[i], buffer[i+1], buffer[i+2], buffer[i+3], buffer[i+4], buffer[i+5], buffer[i+6]);
    i += 7;
  }
  function func9(name) {
    ctx[name](buffer[i], buffer[i+1], buffer[i+2], buffer[i+3], buffer[i+4], buffer[i+5], buffer[i+6], buffer[i+7], buffer[i+8]);
    i += 9;
  }

  // lookuppers, convert integer ids to cached objects for some args
  function func1L0(name) {
    ctx[name](objects[buffer[i]]);
    i++;
  }
  function func2L0(name) {
    ctx[name](objects[buffer[i]], buffer[i+1]);
    i += 2;
  }
  function func2L0L1(name) {
    ctx[name](objects[buffer[i]], objects[buffer[i+1]]);
    i += 2;
  }
  function func2L1_(name) {
    ctx[name](buffer[i], buffer[i+1] ? objects[buffer[i+1]] : null);
    i += 2;
  }
  function func3L0(name) {
    ctx[name](objects[buffer[i]], buffer[i+1], buffer[i+2]);
    i += 3;
  }
  function func4L3_(name) {
    ctx[name](buffer[i], buffer[i+1], buffer[i+2], buffer[i+3] ? objects[buffer[i+3]] : null);
    i += 4;
  }
  function func5L3_(name) {
    ctx[name](buffer[i], buffer[i+1], buffer[i+2], buffer[i+3] ? objects[buffer[i+3]] : null, buffer[i+4]);
    i += 5;
  }

  // constructors, last argument is the id to save as
  function funcC0(name) {
    var object = ctx[name]();
    var id = buffer[i++];
    objects[id] = object;
  }
  function funcC1(name) {
    var object = ctx[name](buffer[i++]);
    var id = buffer[i++];
    objects[id] = object;
  }
  function funcC2(name) {
    var object = ctx[name](buffer[i++], buffer[i++]);
    var id = buffer[i++];
    objects[id] = object;
  }
  function funcC2L0(name) {
    var object = ctx[name](objects[buffer[i++]], buffer[i++]);
    var id = buffer[i++];
    objects[id] = object;
  }

  // destructors, stop holding on to the object in the cache
  function funcD0(name) {
    var id = buffer[i++];
    var object = objects[id];
    objects[id] = null;
    ctx[name](object);
  }

  // special cases/optimizations
  function bindFramebuffer() {
    currFrameBuffer = buffer[i+1] ? objects[buffer[i+1]] : null;
    ctx.bindFramebuffer(buffer[i], currFrameBuffer);
    i += 2;
  }
  function drawArrays(name) {
    if (!skippable || currFrameBuffer !== null) {
      ctx.drawArrays(buffer[i], buffer[i+1], buffer[i+2]);
    }
    i += 3;
  }
  function drawElements(name) {
    if (!skippable || currFrameBuffer !== null) {
      ctx.drawElements(buffer[i], buffer[i+1], buffer[i+2], buffer[i+3]);
    }
    i += 4;
  }
  function enable() {
    ctx.enable(buffer[i++]);
  }
  function disable() {
    ctx.disable(buffer[i++]);
  }
  function enableVertexAttribArray() {
    ctx.enableVertexAttribArray(buffer[i++]);
  }
  function disableVertexAttribArray() {
    ctx.disableVertexAttribArray(buffer[i++]);
  }
  function activeTexture() {
    ctx.activeTexture(buffer[i++]);
  }
  function uniform1i() {
    ctx.uniform1i(objects[buffer[i]], buffer[i+1]);
    i += 2;
  }
  function uniform1f() {
    ctx.uniform1f(objects[buffer[i]], buffer[i+1]);
    i += 2;
  }
  function uniform1fv() {
    ctx.uniform1fv(objects[buffer[i]], buffer[i+1]);
    i += 2;
  }
  function uniform2fv() {
    ctx.uniform2fv(objects[buffer[i]], buffer[i+1]);
    i += 2;
  }
  function uniform1iv() {
    ctx.uniform1iv(objects[buffer[i]], buffer[i+1]);
    i += 2;
  }
  function uniform3f() {
    ctx.uniform3f(objects[buffer[i]], buffer[i+1], buffer[i+2], buffer[i+3]);
    i += 4;
  }
  function uniform3fv() {
    ctx.uniform3fv(objects[buffer[i]], buffer[i+1]);
    i += 2;
  }
  function uniform4fv() {
    ctx.uniform4fv(objects[buffer[i]], buffer[i+1]);
    i += 2;
  }
  function vertexAttribPointer() {
    ctx.vertexAttribPointer(buffer[i], buffer[i+1], buffer[i+2], buffer[i+3], buffer[i+4], buffer[i+5]);
    i += 6;
  }

  var calls = {
    0: { name: 'NULL', func: func0 },
    1: { name: 'getExtension', func: func1 },
    2: { name: 'enable', func: enable },
    3: { name: 'disable', func: disable },
    4: { name: 'clear', func: func1 },
    5: { name: 'clearColor', func: func4 },
    6: { name: 'createShader', func: funcC1 },
    7: { name: 'deleteShader', func: funcD0 },
    8: { name: 'shaderSource', func: func2L0 },
    9: { name: 'compileShader', func: func1L0 },
    10: { name: 'createProgram', func: funcC0 },
    11: { name: 'deleteProgram', func: funcD0 },
    12: { name: 'attachShader', func: func2L0L1 },
    13: { name: 'bindAttribLocation', func: func3L0 },
    14: { name: 'linkProgram', func: func1L0 },
    15: { name: 'getProgramParameter', func: () => assert(ctx.getProgramParameter(objects[buffer[i++]], buffer[i++]), 'we cannot handle errors, we are async proxied WebGL') },
    16: { name: 'getUniformLocation', func: funcC2L0 },
    17: { name: 'useProgram', func: func1L0 },
    18: { name: 'uniform1i', func: uniform1i },
    19: { name: 'uniform1f', func: uniform1f },
    20: { name: 'uniform3fv', func: uniform3fv },
    21: { name: 'uniform4fv', func: uniform4fv },
    22: { name: 'uniformMatrix4fv', func: func3L0 },
    23: { name: 'vertexAttrib4fv', func: func2 },
    24: { name: 'createBuffer', func: funcC0 },
    25: { name: 'deleteBuffer', func: funcD0 },
    26: { name: 'bindBuffer', func: func2L1_ },
    27: { name: 'bufferData', func: func3 },
    28: { name: 'bufferSubData', func: func3 },
    29: { name: 'viewport', func: func4 },
    30: { name: 'vertexAttribPointer', func: vertexAttribPointer },
    31: { name: 'enableVertexAttribArray', func: enableVertexAttribArray },
    32: { name: 'disableVertexAttribArray', func: disableVertexAttribArray },
    33: { name: 'drawArrays', func: drawArrays },
    34: { name: 'drawElements', func: drawElements },
    35: { name: 'getError', func: () => assert(ctx.getError() === ctx.NO_ERROR, 'we cannot handle errors, we are async proxied WebGL') },
    36: { name: 'createTexture', func: funcC0 },
    37: { name: 'deleteTexture', func: funcD0 },
    38: { name: 'bindTexture', func: func2L1_ },
    39: { name: 'texParameteri', func: func3 },
    40: { name: 'texImage2D', func: func9 },
    41: { name: 'compressedTexImage2D', func: func7 },
    42: { name: 'activeTexture', func: activeTexture },
    43: { name: 'getShaderParameter', func: () => assert(ctx.getShaderParameter(objects[buffer[i++]], buffer[i++]), 'we cannot handle errors, we are async proxied WebGL') },
    44: { name: 'clearDepth', func: func1 },
    45: { name: 'depthFunc', func: func1 },
    46: { name: 'frontFace', func: func1 },
    47: { name: 'cullFace', func: func1 },
    48: { name: 'pixelStorei', func: func2 },
    49: { name: 'depthMask', func: func1 },
    50: { name: 'depthRange', func: func2 },
    51: { name: 'blendFunc', func: func2 },
    52: { name: 'scissor', func: func4 },
    53: { name: 'colorMask', func: func4 },
    54: { name: 'lineWidth', func: func1 },
    55: { name: 'createFramebuffer', func: funcC0 },
    56: { name: 'deleteFramebuffer', func: funcD0 },
    57: { name: 'bindFramebuffer', func: bindFramebuffer },
    58: { name: 'framebufferTexture2D', func: func5L3_ },
    59: { name: 'createRenderbuffer', func: funcC0 },
    60: { name: 'deleteRenderbuffer', func: funcD0 },
    61: { name: 'bindRenderbuffer', func: func2L1_ },
    62: { name: 'renderbufferStorage', func: func4 },
    63: { name: 'framebufferRenderbuffer', func: func4L3_ },
    64: { name: 'debugPrint', func: func1 },
    65: { name: 'hint', func: func2 },
    66: { name: 'blendEquation', func: func1 },
    67: { name: 'generateMipmap', func: func1 },
    68: { name: 'uniformMatrix3fv', func: func3L0 },
    69: { name: 'stencilMask', func: func1 },
    70: { name: 'clearStencil', func: func1 },
    71: { name: 'texSubImage2D', func: func9 },
    72: { name: 'uniform3f', func: uniform3f },
    73: { name: 'blendFuncSeparate', func: func4 },
    74: { name: 'uniform2fv', func: uniform2fv },
    75: { name: 'texParameterf', func: func3 },
    76: { name: 'isContextLost', func: () => assert(!ctx.isContextLost(), 'context lost which we cannot handle, we are async proxied WebGL') },
    77: { name: 'blendEquationSeparate', func: func2 },
    78: { name: 'stencilFuncSeparate', func: func4 },
    79: { name: 'stencilOpSeparate', func: func4 },
    80: { name: 'drawBuffersWEBGL', func: func1 },
    81: { name: 'uniform1iv', func: uniform1iv },
    82: { name: 'uniform1fv', func: uniform1fv },
  };

  function renderCommands(buf) {
    ctx = Module.ctx;
    i = 0;
    buffer = buf;
    var len = buffer.length;
    //dump('issuing commands, buffer len: ' + len + '\n');
    while (i < len) {
      var info = calls[buffer[i++]];
      var name = info.name;
      info.func(name);
      //var err;
      //while ((err = ctx.getError()) !== ctx.NO_ERROR) {
      //  dump('warning: GL error ' + err + ', after ' + [command, numArgs] + '\n');
      //}
      assert(i <= len);
    }
  }

  var commandBuffers = [];

  function renderAllCommands() {
    // we can skip parts of the frames before the last, as we just need their side effects
    skippable = true;
    for (var i = 0; i < commandBuffers.length-1; i++) {
      renderCommands(commandBuffers[i]);
    }
    skippable = false;
    renderCommands(commandBuffers[commandBuffers.length-1]);
    commandBuffers.length = 0;
  }

  this.onmessage = (msg) => {
    //dump('client GL got ' + JSON.stringify(msg) + '\n');
    switch (msg.op) {
      case 'render': {
        if (commandBuffers.length === 0) {
          // requestion a new frame, we will clear the buffers after rendering them
          window.requestAnimationFrame(renderAllCommands);
        }
        commandBuffers.push(msg.commandBuffer);
        break;
      }
      default: throw 'weird gl onmessage ' + JSON.stringify(msg);
    }
  };
}

WebGLClient.prefetch = () => {
  // Create a fake temporary GL context
  var canvas = document.createElement('canvas');
  var ctx = canvas.getContext('webgl-experimental') || canvas.getContext('webgl');
  if (!ctx) {
    // If we have no webGL support, we still notify that prefetching is done, as the app blocks on that
    worker.postMessage({ target: 'gl', op: 'setPrefetched', preMain: true });
    return;
  }

  // Fetch the parameters and proxy them
  var parameters = {};
  ['MAX_VERTEX_ATTRIBS', 'MAX_TEXTURE_IMAGE_UNITS', 'MAX_TEXTURE_SIZE', 'MAX_CUBE_MAP_TEXTURE_SIZE', 'MAX_VERTEX_UNIFORM_VECTORS', 'MAX_FRAGMENT_UNIFORM_VECTORS',
   'MAX_VARYING_VECTORS', 'MAX_COMBINED_TEXTURE_IMAGE_UNITS', 'MAX_VERTEX_TEXTURE_IMAGE_UNITS', 'VENDOR', 'RENDERER', 'VERSION', 'SHADING_LANGUAGE_VERSION',
   'COMPRESSED_TEXTURE_FORMATS', 'RED_BITS', 'GREEN_BITS', 'BLUE_BITS', 'ALPHA_BITS', 'DEPTH_BITS', 'STENCIL_BITS', 'MAX_RENDERBUFFER_SIZE'].forEach(function(name) {
    var id = ctx[name];
    parameters[id] = ctx.getParameter(id);
  });
  // Try to enable some extensions, so we can access their parameters
  [{ extName: 'EXT_texture_filter_anisotropic', paramName: 'MAX_TEXTURE_MAX_ANISOTROPY_EXT' },
   { extName: 'WEBGL_draw_buffers', paramName: 'MAX_COLOR_ATTACHMENTS_WEBGL' }].forEach(function(pair) {
    var ext = ctx.getExtension(pair.extName);
    if (ext) {
      var id = ext[pair.paramName];
      parameters[id] = ctx.getParameter(id);
    }
  });
  // Fetch shader precisions
  var precisions = {};
  ['FRAGMENT_SHADER', 'VERTEX_SHADER'].forEach(function(shaderType) {
    shaderType = ctx[shaderType];
    precisions[shaderType] = {};
    ['LOW_FLOAT', 'MEDIUM_FLOAT', 'HIGH_FLOAT', 'LOW_INT', 'MEDIUM_INT', 'HIGH_INT'].forEach(function(precisionType) {
      precisionType = ctx[precisionType];
      var info = ctx.getShaderPrecisionFormat(shaderType, precisionType);
      precisions[shaderType][precisionType] = info ? { rangeMin: info.rangeMin, rangeMax: info.rangeMax, precision: info.precision } : info;
    });
  });

  worker.postMessage({ target: 'gl', op: 'setPrefetched', parameters, extensions: ctx.getSupportedExtensions(), precisions, preMain: true });
};


/**
 * @license
 * Copyright 2013 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

/*
 * Proxy events/work to/from an emscripen worker built
 * with PROXY_TO_WORKER.  This code runs on the main
 * thread and is not part of the main emscripten output
 * file.
 */

var ENVIRONMENT_IS_NODE = typeof process == 'object' && typeof process.versions == 'object' && typeof process.versions.node == 'string';
if (ENVIRONMENT_IS_NODE) {
  global.Worker = require('worker_threads').Worker;
  var Module = Module || {}
} else
if (typeof Module == 'undefined') {
  console.warn('no Module object defined - cannot proxy canvas rendering and input events, etc.');
  Module = {
    canvas: {
      addEventListener: () => {},
      getBoundingClientRect: () => ({ bottom: 0, height: 0, left: 0, right: 0, top: 0, width: 0 }),
    },
  };
}

if (!Module.hasOwnProperty('print')) {
  Module['print'] = (x) => console.log(x);
}

if (!Module.hasOwnProperty('printErr')) {
  Module['printErr'] = (x) => console.error(x);
}

// utils

function FPSTracker(text) {
  var last = 0;
  var mean = 0;
  var counter = 0;
  this.tick = () => {
    var now = Date.now();
    if (last > 0) {
      var diff = now - last;
      mean = 0.99*mean + 0.01*diff;
      if (counter++ === 60) {
        counter = 0;
        dump(text + ' fps: ' + (1000/mean).toFixed(2) + '\n');
      }
    }
    last = now;
  };
}

/*
function GenericTracker(text) {
  var mean = 0;
  var counter = 0;
  this.tick = (value) => {
    mean = 0.99*mean + 0.01*value;
    if (counter++ === 60) {
      counter = 0;
      dump(text + ': ' + (mean).toFixed(2) + '\n');
    }
  };
}
*/

// render

var renderFrameData = null;

function renderFrame() {
  var dst = Module.canvasData.data;
  if (dst.set) {
    dst.set(renderFrameData);
  } else {
    for (var i = 0; i < renderFrameData.length; i++) {
      dst[i] = renderFrameData[i];
    }
  }
  Module.ctx.putImageData(Module.canvasData, 0, 0);
  renderFrameData = null;
}

if (typeof window != 'undefined') {
  window.requestAnimationFrame = window.requestAnimationFrame || window.mozRequestAnimationFrame ||
                                 window.webkitRequestAnimationFrame || window.msRequestAnimationFrame ||
                                 renderFrame;
}

/*
(function() {
  var trueRAF = window.requestAnimationFrame;
  var tracker = new FPSTracker('client');
  window.requestAnimationFrame = (func) => {
    trueRAF(() => {
      tracker.tick();
      func();
    });
  }
})();
*/

// end render

// IDBStore

// include: IDBStore.js
/**
 * @license
 * Copyright 2015 The Emscripten Authors
 * SPDX-License-Identifier: MIT
 */

var IDBStore = {
  indexedDB() {
    if (typeof indexedDB != 'undefined') return indexedDB;
    var ret = null;
    if (typeof window == 'object') ret = window.indexedDB || window.mozIndexedDB || window.webkitIndexedDB || window.msIndexedDB;
    assert(ret, 'IDBStore used, but indexedDB not supported');
    return ret;
  },
  DB_VERSION: 22,
  DB_STORE_NAME: 'FILE_DATA',
  dbs: {},
  blobs: [0],
  getDB(name, callback) {
    // check the cache first
    var db = IDBStore.dbs[name];
    if (db) {
      return callback(null, db);
    }
    var req;
    try {
      req = IDBStore.indexedDB().open(name, IDBStore.DB_VERSION);
    } catch (e) {
      return callback(e);
    }
    req.onupgradeneeded = (e) => {
      var db = /** @type {IDBDatabase} */ (e.target.result);
      var transaction = e.target.transaction;
      var fileStore;
      if (db.objectStoreNames.contains(IDBStore.DB_STORE_NAME)) {
        fileStore = transaction.objectStore(IDBStore.DB_STORE_NAME);
      } else {
        fileStore = db.createObjectStore(IDBStore.DB_STORE_NAME);
      }
    };
    req.onsuccess = () => {
      db = /** @type {IDBDatabase} */ (req.result);
      // add to the cache
      IDBStore.dbs[name] = db;
      callback(null, db);
    };
    req.onerror = function(event) {
      callback(event.target.error || 'unknown error');
      event.preventDefault();
    };
  },
  getStore(dbName, type, callback) {
    IDBStore.getDB(dbName, (error, db) => {
      if (error) return callback(error);
      var transaction = db.transaction([IDBStore.DB_STORE_NAME], type);
      transaction.onerror = (event) => {
        callback(event.target.error || 'unknown error');
        event.preventDefault();
      };
      var store = transaction.objectStore(IDBStore.DB_STORE_NAME);
      callback(null, store);
    });
  },
  // External API
  getFile(dbName, id, callback) {
    IDBStore.getStore(dbName, 'readonly', (err, store) => {
      if (err) return callback(err);
      var req = store.get(id);
      req.onsuccess = (event) => {
        var result = event.target.result;
        if (!result) {
          return callback(`file ${id} not found`);
        }
        return callback(null, result);
      };
      req.onerror = callback;
    });
  },
  setFile(dbName, id, data, callback) {
    IDBStore.getStore(dbName, 'readwrite', (err, store) => {
      if (err) return callback(err);
      var req = store.put(data, id);
      req.onsuccess = (event) => callback();
      req.onerror = callback;
    });
  },
  deleteFile(dbName, id, callback) {
    IDBStore.getStore(dbName, 'readwrite', (err, store) => {
      if (err) return callback(err);
      var req = store.delete(id);
      req.onsuccess = (event) => callback();
      req.onerror = callback;
    });
  },
  existsFile(dbName, id, callback) {
    IDBStore.getStore(dbName, 'readonly', (err, store) => {
      if (err) return callback(err);
      var req = store.count(id);
      req.onsuccess = (event) => callback(null, event.target.result > 0);
      req.onerror = callback;
    });
  },
  clearStore(dbName, callback) {
    IDBStore.getStore(dbName, 'readwrite', (err, store) => {
      if (err) return callback(err);
      var req = store.clear();
      req.onsuccess = (event) => callback();
      req.onerror = callback;
    });
  },
};
// end include: IDBStore.js
// Frame throttling

var frameId = 0;

// Temporarily handling this at run-time pending Python preprocessor support

var SUPPORT_BASE64_EMBEDDING;

// Worker

var filename;
filename ||= './glpsol.worker.js';

var workerURL = filename;
if (SUPPORT_BASE64_EMBEDDING) {
  var fileBytes = tryParseAsDataURI(filename);
  if (fileBytes) {
    workerURL = URL.createObjectURL(new Blob([fileBytes], {type: 'application/javascript'}));
  }
}
var worker = new Worker(workerURL);

if (ENVIRONMENT_IS_NODE) {
  worker.postMessage({target: 'worker-init'});
} else {
WebGLClient.prefetch();

setTimeout(() => {
  worker.postMessage({
    target: 'worker-init',
    width: Module.canvas.width,
    height: Module.canvas.height,
    boundingClientRect: cloneObject(Module.canvas.getBoundingClientRect()),
    URL: document.URL,
    currentScriptUrl: filename,
    preMain: true });
}, 0); // delay til next frame, to make sure html is ready
}

var workerResponded = false;

worker.onmessage = (event) => {
  //dump('\nclient got ' + JSON.stringify(event.data).substr(0, 150) + '\n');
  if (!workerResponded) {
    workerResponded = true;
    Module.setStatus?.('');
    if (SUPPORT_BASE64_EMBEDDING && workerURL !== filename) URL.revokeObjectURL(workerURL);
  }

  var data = event.data;
  switch (data.target) {
    case 'stdout': {
      Module['print'](data.content);
      break;
    }
    case 'stderr': {
      Module['printErr'](data.content);
      break;
    }
    case 'window': {
      window[data.method]();
      break;
    }
    case 'canvas': {
      switch (data.op) {
        case 'getContext': {
          Module.ctx = Module.canvas.getContext(data.type, data.attributes);
          if (data.type !== '2d') {
            // possible GL_DEBUG entry point: Module.ctx = wrapDebugGL(Module.ctx);
            Module.glClient = new WebGLClient();
          }
          break;
        }
        case 'resize': {
          Module.canvas.width = data.width;
          Module.canvas.height = data.height;
          if (Module.ctx?.getImageData) Module.canvasData = Module.ctx.getImageData(0, 0, data.width, data.height);
          worker.postMessage({ target: 'canvas', boundingClientRect: cloneObject(Module.canvas.getBoundingClientRect()) });
          break;
        }
        case 'render': {
          if (renderFrameData) {
            // previous image was not rendered yet, just update image
            renderFrameData = data.image.data;
          } else {
            // previous image was rendered so update image and request another frame
            renderFrameData = data.image.data;
            window.requestAnimationFrame(renderFrame);
          }
          break;
        }
        case 'setObjectProperty': {
          Module.canvas[data.object][data.property] = data.value;
          break;
        }
        default: throw 'eh?';
      }
      break;
    }
    case 'gl': {
      Module.glClient.onmessage(data);
      break;
    }
    case 'tick': {
      frameId = data.id;
      worker.postMessage({ target: 'tock', id: frameId });
      break;
    }
    case 'Image': {
      assert(data.method === 'src');
      var img = new Image();
      img.onload = () => {
        assert(img.complete);
        var canvas = document.createElement('canvas');
        canvas.width = img.width;
        canvas.height = img.height;
        var ctx = canvas.getContext('2d');
        ctx.drawImage(img, 0, 0);
        var imageData = ctx.getImageData(0, 0, img.width, img.height);
        worker.postMessage({ target: 'Image', method: 'onload', id: data.id, width: img.width, height: img.height, data: imageData.data, preMain: true });
      };
      img.onerror = () => {
        worker.postMessage({ target: 'Image', method: 'onerror', id: data.id, preMain: true });
      };
      img.src = data.src;
      break;
    }
    case 'IDBStore': {
      switch (data.method) {
        case 'loadBlob': {
          IDBStore.getFile(data.db, data.id, (error, blob) => {
            worker.postMessage({
              target: 'IDBStore',
              method: 'response',
              blob: error ? null : blob
            });
          });
          break;
        }
        case 'storeBlob': {
          IDBStore.setFile(data.db, data.id, data.blob, (error) => {
            worker.postMessage({
              target: 'IDBStore',
              method: 'response',
              error: !!error
            });
          });
          break;
        }
      }
      break;
    }
    case 'custom': {
      if (Module['onCustomMessage']) {
        Module['onCustomMessage'](event);
      } else {
        throw 'Custom message received but client Module.onCustomMessage not implemented.';
      }
      break;
    }
    case 'setimmediate': {
      worker.postMessage({target: 'setimmediate'});
      break;
    }
    default: throw 'what? ' + data.target;
  }
};

function postCustomMessage(data, options = {}) {
  worker.postMessage({ target: 'custom', userData: data, preMain: options.preMain });
}

function cloneObject(event) {
  var ret = {};
  for (var x in event) {
    if (x == x.toUpperCase()) continue;
    var prop = event[x];
    if (typeof prop == 'number' || typeof prop == 'string') ret[x] = prop;
  }
  return ret;
};

if (!ENVIRONMENT_IS_NODE) {

// Only prevent default on backspace/tab because we don't want unexpected navigation.
// Do not prevent default on the rest as we need the keypress event.
function shouldPreventDefault(event) {
  if (event.type === 'keydown' && event.keyCode !== 8 /* backspace */ && event.keyCode !== 9 /* tab */) {
    return false; // keypress, back navigation
  } else {
    return true; // NO keypress, NO back navigation
  }
};

['keydown', 'keyup', 'keypress', 'blur', 'visibilitychange'].forEach((event) => {
  document.addEventListener(event, (event) => {
    worker.postMessage({ target: 'document', event: cloneObject(event) });

    if (shouldPreventDefault(event)) {
      event.preventDefault();
    }
  });
});

['unload'].forEach((event) => {
  window.addEventListener(event, (event) => {
    worker.postMessage({ target: 'window', event: cloneObject(event) });
  });
});

['mousedown', 'mouseup', 'mousemove', 'DOMMouseScroll', 'mousewheel', 'mouseout'].forEach((event) => {
  Module.canvas.addEventListener(event, (event) => {
    worker.postMessage({ target: 'canvas', event: cloneObject(event) });
    event.preventDefault();
  }, true);
});

}
