var editor = (function() {
	var code;
	var command_line;
	var output;
	var compile;
	
	var httpRequest;

  function makeRequest(url, input) {
    
    
  }

	function process_remote_impl(cmd) {
		output.text('');
		var to_compile = {
			"src": code.getValue(),
			"cmd": cmd,
		};

        if (location.protocol === 'https:') {
            // page is secure, c
            location.href = 'http:' + window.location.href.substring(window.location.protocol.length);
        }

		output.text("Executing... Please wait.");

        if (window.XMLHttpRequest) { // Mozilla, Safari, ...
          httpRequest = new XMLHttpRequest();
        } else if (window.ActiveXObject) { // IE
          try {
            httpRequest = new ActiveXObject("Msxml2.XMLHTTP");
          } 
          catch (e) {
            try {
              httpRequest = new ActiveXObject("Microsoft.XMLHTTP");
            } 
            catch (e) {}
          }
        }

        if (!httpRequest) {
          alert('Giving up :( Cannot create an XMLHTTP instance');
          return false;
        }
        
        httpRequest.onreadystatechange = function(){
            if (httpRequest.readyState === 4) {
              if (httpRequest.status === 200) {
                output.text(httpRequest.responseText);
              } else {
                output.text("Server error: " + httpRequest.responseText);
              }
            }
        };

        httpRequest.open('POST', 'http://coliru.stacked-crooked.com/compile');
        httpRequest.setRequestHeader('Content-Type', 'text/plain; charset=utf-8');
        httpRequest.send(JSON.stringify(to_compile));
	};

	function compile_impl() {
		process_remote_impl(
			compile.val() + " && echo 'Compilation: SUCCESS' "
		);
	};

	function run_impl() {
		if (!command_line.val()) {
			command_line.val("");
		}
		process_remote_impl(
			compile.val()
			+ " && echo 'Compilation: SUCCESS. Program output:\n' && ./a.out " + command_line.val() + " && echo \"\nExit code: $?\""
		);
	};

	function init_impl(code_block, command_line_block, output_block, compile_block) {
		code = code_block;
		command_line = command_line_block;
		output = output_block;
		compile = compile_block;

		code.setTheme("ace/theme/textmate");
		code.getSession().setMode("ace/mode/c_cpp");
		code.setShowPrintMargin(false);
		code.setOptions({
			maxLines: Infinity,
			fontSize: "12pt",
		});
		code.$blockScrolling = Infinity;
	};

	return {
		compile: compile_impl,
		run: run_impl,
		init: init_impl,
	};

})();

