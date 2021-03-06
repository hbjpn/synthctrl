var argv = require('optimist').argv;
var cp = require('child_process');
var express = require('express');
var app = express();
var port = argv.port || 8081;
var datadir = argv.datadir || "../data/";
// Authentication for Facebook

console.log("port set to " + port);
console.log("datadir set to " + datadir);
console.log(argv);
app.set("view engine", "ejs");
//app.use(sessionMiddleware);
//app.use(cookieParser());
//app.use(bodyParser());
//app.use(methodOverride());


app.get('/', function(req, res){
  res.render('index');
});
function remove(array, val)
{
	for(var i = 0; i < array.length; ++i){
		if(array[i] == val){
			array.splice(i,1);
			break;
		}
	}
}

app.use(express.static(__dirname));
app.get("/test", function(req,res){
	res.send(req.user);
});

var http = require('http').Server(app);
http.listen(port);
console.log("Listen on port " + port);
var io = require('socket.io').listen(http);








rooms = {};
contentList = {};


var websocket = null;

var fs = require("fs");



var net = require('net');
var dgram = require('dgram');
var ENGINE_HOST = '127.0.0.1';
var ENGINE_PORT = 6565;

var message = new Buffer("Hoge");
var sockclient = dgram.createSocket('udp4');
sockclient.send(message, 0, message.length, ENGINE_PORT, ENGINE_HOST, function(err, bytes){
	if(err) console.log(err);
	console.log("Message sent!!");
});

sockclient.on('message', function(msg, rinfo){
    msgs = msg.toString();
    console.log("Message received from local server : " + msg.toString());
    if(websocket){
        var type = msgs.split(" ")[0];
        var arg = msgs.split(" ")[1] - 0;
        var obj = {type:type, index:arg};
        console.log("Send notification");
        console.log(obj);
        websocket.emit('notify',obj);
    }
});

function GUID(){
	var streamId = "";
	for(var i = 0; i < 4; ++i){
		var s = ("00000000" + (parseInt(Math.floor(Math.random()*0xFFFFFFFF)).toString(16))).slice(-8);
		streamId += s;
	}
	return streamId;
}

RPCBody = {
	'list' : function(socket, data, res){

		fs.readdir(datadir, function(err, files){
				if (err) throw err;
				var fileList = [];
				files.filter(function(file){
					var path = datadir + "/" + file;
					return fs.statSync(path).isFile() && /.*\.json$/.test(file); //絞り込み
					}).forEach(function (file) {
						fileList.push(file);
						});
				console.log(fileList);
				res.send({error:false, files:fileList}); 
				});
	},
	'load' : function(socket, data, res){

		fs.readFile(datadir+"/"+data, "utf8", function(err, text){
				if (err) throw err;
				res.send({error:false, name:data, content:text}); 
				});
	},
	'save' : function(socket, data, res){

		fs.writeFile(datadir+"/"+data.name, data.content, function(err){
				if (err) res.send({error:true,msg:err});
				else res.send({error:false});
				});

	},
	'remove' : function(socket, data, res){
		fs.unlink(datadir+"/"+data.name, function(err){
			if(err) res.send({error:true,msg:err});
			else res.send({error:false});
		});
	},
	'deploy' : function(socket, data, res){
		var message = new Buffer("deploy "+(datadir+"/"+data.name));
		sockclient.send(message, 0, message.length, ENGINE_PORT, ENGINE_HOST, function(err, bytes){
			if(err) res.send({error:true});
			else res.send({error:false});
		});
	},
	'gpio0' : function(socket, data, res){
		var message = new Buffer("gpio0");
		sockclient.send(message, 0, message.length, ENGINE_PORT, ENGINE_HOST, function(err, bytes){
			if(err) console.log(err);
			console.log("Message sent!!");
		});
	},
	'gpio1' : function(socket, data, res){
		var message = new Buffer("gpio1");
		sockclient.send(message, 0, message.length, ENGINE_PORT, ENGINE_HOST, function(err, bytes){
			if(err) console.log(err);
			console.log("Message sent!!");
		});
	},
    'command' : function(socket, data, res){
        cp.exec(data.command, function(err, stdout, stderr){
            console.log("Executing : " + data.command);
            res.send({error:err?true:false, stdout:stdout, stderr:stderr});
        });
    }

};

function RPCResponse(socket, rpcid)
{
	this.socket = socket;
	this.rpcid = rpcid;
	this.sent = false;
}

RPCResponse.prototype.send = function(resbody){
	if(!this.sent)
		this.socket.emit('RPC', {RPCID:this.rpcid,error:false,data:resbody});
	this.sent = true;
};

function cleanup()
{
	console.log("Graceful shutdown ... ");
	process.exit(0);
}

process.on('SIGTERM', cleanup);
process.on('SIGINT', cleanup);

io.sockets.
  //use(function(socket,next){ sessionMiddleware(socket.request, {}, next); }).
  on('connection', function (socket) {
    websocket = socket;
    //console.log("Socket user id = " + JSON.stringify(socket.request.session.passport));
  socket.custom = {};
  socket.on('RPC', function(req) {
	  console.log('RPC call');
	  console.log(req);
	 if(req.evt in RPCBody){
		var res = new RPCResponse(socket, req.RPCID);
		 RPCBody[req.evt](socket, req.data, res);
	 }else{
		 socket.emit('RPC',{RPCID:req.RPCID,error:true});
	 }
  });
  socket.on('disconnect', function (){
    console.log("disconnect!!");
  });
});

