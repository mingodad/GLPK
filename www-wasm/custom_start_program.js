
var statusElement = document.getElementById('status');
var progressElement = document.getElementById('progress');
var spinnerElement = document.getElementById('spinner');
var start_button = document.getElementById("start_button");
var output_element = document.getElementById('output');
if (typeof term === "undefined") {
  term = null;
}

if (start_button != null) {
  start_button.disabled = true;
}
if (output_element != null) {
  output_element.readOnly = true;
}

Module['setStatus'] =
  function(text) {
    statusElement.innerHTML = text;

    if (start_button != null) {
      if (!text) {
        start_button.disabled = false;
      } else {
        start_button.disabled = true;
      }
    }
  };


function remove_terminal_codes(str) {
  // http://stackoverflow.com/questions/25245716/remove-all-ansi-colors-styles-from-strings
  return str.replace(/[\u001b\u009b][[()#;?]*(?:[0-9]{1,4}(?:;[0-9]{0,4})*)?[0-9A-ORZcf-nqry=><]/g, '');
}

function remove_backspaces(str) {
  while (str.indexOf("\b") != -1) {
    str = str.replace(/.?[\b]/, "");
  }
  return str;
}

print_function = 
  (function() {
    var element = document.getElementById('output');
    if (element) element.value = ''; // clear browser cache
    return function(text) {
      if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
      // These replacements are necessary if you render to raw HTML
      //text = text.replace(/&/g, "&amp;");
      //text = text.replace(/</g, "&lt;");
      //text = text.replace(/>/g, "&gt;");
      //text = text.replace('\n', '<br>', 'g');
      console.log(text);  // Log the raw text received.
      if (term) {
        term.writeln(text);
      } 
      if (element) {
        text = remove_terminal_codes(text);
        text = remove_backspaces(text);
        element.value += text + "\n";
        element.scrollTop = element.scrollHeight; // focus on bottom
      }
    };
  })();

Module['print'] = print_function;
Module['printErr'] = print_function;

Module['onCustomMessage'] =
  function(event) {
    var data = event.data;
    if (data.command == "callMainComplete") {
      postCustomMessage({command: "FS_sync"}, {preMain: false});
      Module['setStatus']('');
    }
  };


var created_file = false;

var start_program = function() {
  start_button.disabled = true;
  if (output_element != null) {
    output_element.value = "";
  }
  if (term) {
    term.clear();
  }

  Module['setStatus']('Running...');

  var input_file_content = null;
  var input_field = document.getElementById("input");
  if (input_field != null) {
    input_file_content = input_field.value;
  }

  var command_line_content = [];
  var command_line = document.getElementById("command_line");
  if (command_line != null) {
    command_line_content = command_line.value.split(" ").filter(function(el) {return el.length != 0});
  }

  console.warn("Start program with " + command_line_content);

  if (input_file_content) {
    if (created_file) {
      postCustomMessage({command: "FS_unlink", filename: "./input_file"}, {preMain: true});
    }
    postCustomMessage({command: "FS_createDataFile", dir: ".", filename: "input_file", content: input_file_content}, {preMain: true});
    created_file = true;
  }

  postCustomMessage({command: "callMain", arguments: command_line_content}, {preMain: true});
};

setTimeout(
  function() {
    postCustomMessage({command: "custom-init"}, {preMain: true});
  }, 0);
