$(function () {
    $.jsPanel({
        headerTitle: "Console",
        theme: "rebeccapurple",
        headerControls: {
            close: 'remove'
        },
        content: "",
        callback: function () {
            this.content.attr("id","console");
            this.content.css("color","#aaa");
            this.content.css("background-color","#000");
            var id = 1;
            $('#console').terminal(function(command, term) {
                if (command == 'help') {
                    term.echo("available commands are mysql, js, test");
                } else if (command == 'test'){
                    term.push(function(command, term) {
                        if (command == 'help') {
                            term.echo('if you type ping it will display pong');
                        } else if (command == 'ping') {
                            term.echo('pong');
                        } else {
                            term.echo('unknown command ' + command);
                        }
                    }, {
                        prompt: 'test> ',
                        name: 'test'});
                } else if (command == "js") {
                    term.push(function(command, term) {
                        var result = window.eval(command);
                        if (result != undefined) {
                            term.echo(String(result));
                        }
                    }, {
                        name: 'js',
                        prompt: 'js> '});
                } else if (command == 'mysql') {
                    term.push(function(command, term) {
                        term.pause();
                        $.jrpc("mysql-rpc-demo.php",
                               "query",
                               [command],
                               function(data) {
                                   term.resume();
                                   if (data.error && data.error.message) {
                                       term.error(data.error.message);
                                   } else {
                                       if (typeof data.result == 'boolean') {
                                           term.echo(data.result ? 'success' : 'fail');
                                       } else {
                                           var len = data.result.length;
                                           for(var i=0;i<len; ++i) {
                                               term.echo(data.result[i].join(' | '));
                                           }
                                       }
                                   }
                               },
                               function(xhr, status, error) {
                                   term.error('[AJAX] ' + status +
                                              ' - Server reponse is: \n' +
                                              xhr.responseText);
                                   term.resume();
                               });
                    }, {
                        greetings: "This is example of using mysql from terminal\n\
        you are allowed to execute: select, insert, update and delete from/to table:\n\
            table test(integer_value integer, varchar_value varchar(255))",
                        prompt: "mysql> "});
                } else {
                    term.echo("unknow command " + command);
                }
            }, {
                greetings: "multiply terminals demo use help to see available commands",
                onBlur: function() {
                    // prevent loosing focus
                    return false;
                }
            });
        }
    });

    $.jsPanel({
        headerTitle: "Logcat",
        theme: "blue",
        headerControls: {
            close: 'remove'
        },
        position: {
            left:   10,
            bottom: 10
        },
        content: "",
        callback: function () {
          this.content.attr("id", "logcat");
          this.content.css("color","#aaa");
          this.content.css("background-color","#000");
          window.WebSocket = window.WebSocket || window.MozWebSocket;

          var logcat = $("#logcat");
          // if browser doesn't support WebSocket, just show some notification and exit
          if (!window.WebSocket) {
            logcat.html($('<p>', { text: 'Sorry, but your browser doesn\'t support WebSockets.'} ));
            return;
          }
          // open connection
          var connection = new WebSocket('ws://127.0.0.1:7000');

          connection.onopen = function () {
            logcat.html($('<p>', { text: 'Application connected.'} ));
            connection.send("Hello!");
          };

          connection.onerror = function (error) {
              // just in there were some problems with conenction...
            logcat.prepend($('<p>', { text: 'Sorry, but there\'s some problem with your connection or the server is down.' } ));
          };

          setInterval(function() {
            if (connection.readyState != 1) {
              logcat.prepend($('<p>', { text: 'Unable to comminucate with the WebSocket server.'} ));
            }
          }, 3000);

          var ELogDefault = 0;
          var ELogDebug = 1;
          var ELogInfo = 2;
          var ELogWarn = 3;
          var ELogError = 4;
          var ELogFatal = 5;
          // most important part - incoming messages
          connection.onmessage = function (message) {
            try {
              logitem = JSON.parse(message.data);
              var color = 'white';
              switch (logitem.LogLevel) {
                case ELogFatal:
                  color = 'red';
                  break;
                case ELogError:
                  color = 'red';
                  break;
                case ELogInfo:
                  color = 'green';
                  break;
                default:
                  break;
              }
              logcat.prepend('<span style="color:' + color + ';">' + logitem.Tag + '</span>' + logitem.Log + '<br/>');
            } catch (err) {
              logcat.prepend('<p><span style="color:red;">' + err + '</span>' + '</p>');
            }
          };
        }
    });
});
