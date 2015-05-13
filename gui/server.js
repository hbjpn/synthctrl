var argv = require('optimist').argv;
var express = require('express');
var app = express();
var http = require('http').Server(app);
var io = require('socket.io').listen(http);
var session = require('express-session');
var bodyParser = require("body-parser")
  , cookieParser = require("cookie-parser")
  , methodOverride = require('method-override');
// Set up the Session middleware using a MongoDB session store
var expressSession = require("express-session");
var sessionMiddleware = expressSession({
    name: "sessioncookie",
    secret: "hogefugahoge",
    store: new expressSession.MemoryStore()
});


var port = argv.port || 8081;

// Authentication for Facebook

console.log(argv);
app.set("view engine", "ejs");
app.use(sessionMiddleware);
app.use(cookieParser());
app.use(bodyParser());
app.use(methodOverride());


app.get('/', function(req, res){
  res.render('index');
});

app.get('/studio', function(req, res){
  if(req.user == undefined){
    res.render('studio', { user: '', login_menu:'<a href="/auth/facebook">login</a>'});
  }else{
    res.render('studio', { user: '<a href="#" onclick="onProfileDialog()">'+'Profile'+'</a>', login_menu:'<a href="/logout">logout</a>' });
  }
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
function newroom()
{
	return {member:[], info:null, delTimer:null};
}

function joinroom(socket, room)
{
	socket.join(room);
	//if(!(room in rooms)){ console.log("ERROR"); return ; }
	//rooms[room].member.push(socket.custom.peer_id);
	mongo.db.collection('rooms').update({room_id:room},{$addToSet : {'member' : socket.custom.peer_id}});
	clearDeleteTimer(room);
	socket.custom.room = room;
	socket.broadcast.to(socket.custom.room).emit('peer_joined',{peer:socket.custom.peer_id});
}

function clearDeleteTimer(room_id)
{
	/*
	if(room_id in rooms){
		if(rooms[room_id].delTimer){
			console.log("Clear delete timer for room" + room_id);
			clearTimeout(rooms[room_id].delTimer);
		}
		rooms[room_id].delTimer = null;
	}*/
}

function setDeleteTimer(room_id)
{
/*
	console.log("Set delete timer for room" + room_id);
	clearDeleteTimer(room_id);
	rooms[room_id].delTimer = setTimeout( function(){
		if(room_id in rooms){
			delete rooms[room_id];
			console.log("Room deleted due to timeout : "+room_id);
		}
	}, 24*3600*1000); // Deleted after 24Hours
*/
}

function leaveroom(socket)
{
	if(socket.custom.room){
		socket.broadcast.to(socket.custom.room).emit('peer_left',{peer:socket.custom.peer_id});
		var room_id = socket.custom.room;
			socket.leave(room_id);
		mongo.db.collection('rooms').update({room_id:room_id},{$pull: {"member" : socket.custom.peer_id}}, function(err,item){
			
		});
		//remove(rooms[room_id].member, socket.custom.peer_id);
			//if(rooms[room_id].member.length == 0){
		//	setDeleteTimer(room_id);
		//}
	}
}

app.use(express.static(__dirname));
app.get("/test", function(req,res){
	res.send(req.user);
});

http.listen(port);
console.log("Listen on port " + port);

rooms = {};
contentList = {};

function GUID(){
	var streamId = "";
	for(var i = 0; i < 4; ++i){
		var s = ("00000000" + (parseInt(Math.floor(Math.random()*0xFFFFFFFF)).toString(16))).slice(-8);
		streamId += s;
	}
	return streamId;
}

RPCBody = {
	'join' : function(socket, data, res){
		leaveroom(socket);
		joinroom(socket, data.room);
		//contentList[data.room] = contentList[data.room] || {};
		setTimeout(function(){
			mongo.db.collection('rooms').findOne({room_id:data.room},function(err,room){
				cl = room.content_list || {};
				socket.emit('content_list_update', cl);
			});
		},1); // To ensure order of RPC
		console.log(rooms);
		
		mongo.db.collection('rooms').findOne({room_id : data.room}, function(err,room){
			res.send({error:false, peers:room.member,info:room.info}); 
		}); 
		//return {error:false,peers:rooms[data.room].member, info:rooms[data.room].info};
	},
	'list_room' : function(socket, data, res){
		mongo.db.collection('rooms').find({}).toArray(function(err, rooms){
			var ret = [];
			for(var i = 0; i < rooms.length; ++i){
				var ri = rooms[i].info;
				ri.id = rooms[i].room_id;
				ret.push(ri);
			}
			res.send({error:false,rooms:ret});
		});
	},
	'create_room' : function(socket, req, res){
		{
			var id = GUID();
			//rooms[id] = newroom();
			//rooms[id].info = req;
			mongo.db.collection('rooms').update({room_id:id},{room_id:id, info:req, member:[],  content_list:{}},{upsert:true},function(err,ret){ console.log([err,ret]); });
			//console.log(rooms);
			setDeleteTimer(id);
			res.send({error:false, id:id});
		}
	},
	'publish' : function(socket, data, res){
		  console.log("publish accepted : ");
		  console.log(data);
		  contentId = GUID(); // Generate content ID

		  // Additional information added
		  data.contentId = contentId;
		  data.owner = socket.request.session.passport.user;
		  
		  var tmpVal = {};
		  tmpVal["content_list."+contentId] = data;
		  mongo.db.collection('rooms').update({room_id:socket.custom.room},{$set:tmpVal},function(err,res){
		  	console.log([err,res]);
		  });
		  setTimeout(function(){
			  mongo.db.collection('rooms').findOne({room_id:socket.custom.room},function(err,item){
				console.log("content_list_update notofication sent");
				console.log(item.content_list);
			  	socket.emit('content_list_update', item.content_list);  // Because broadcast dont send to publisher itself.
			  	socket.broadcast.to(socket.custom.room).emit('content_list_update', item.content_list); 
			  });  
	      	  },1); // To ensure order of RPC
		  res.send({error:false, contentId:contentId});
	},
	'getprof' : function(socket, data, res){
		console.log('getprof');
		mongo.db.collection('users').findOne({user_id:data?data:socket.request.session.passport.user}, function(err,item){
			if(err || (!item)){
				res.send({nickname:'Player', profile:'I love music'});
			}else
			{
				res.send(item.profile);
			}
		});
	},
	'setprof' : function(socket, data, res){
		console.log('setprof : ' + JSON.stringify(data));
		var user_id = socket.request.session.passport.user;
		mongo.db.collection('users').update({user_id:user_id}, {user_id:user_id,profile:data}, {upsert:true}, function(err,ret){ console.log([err,ret]);});
		res.send({error:false});
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
	mongo.db.collection('rooms').update({},{$set:{member:[], content_list:{}}},{multi:true}, function(err,res){
		console.log("Clear member and content list from database done");
		process.exit(0);
	});
}

process.on('SIGTERM', cleanup);
process.on('SIGINT', cleanup);

io.sockets.
  use(function(socket,next){ sessionMiddleware(socket.request, {}, next); }).
  on('connection', function (socket) {
    console.log("Socket user id = " + JSON.stringify(socket.request.session.passport));
  socket.custom = {};
  socket.emit('news', { hello: 'world' });
  socket.on('hello', function (data) {
	  console.log(data);
	  socket.custom.peer_id = data.peer_id;
	  socket.custom.room = null; 
  });
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
  /*socket.on('join', function (req) {
	  data = req.data;
	  leaveroom(socket);
	  joinroom(socket, data.room, req.RPCID);
	  contentList[data.room] = contentList[data.room] || {};
	  socket.emit('content_list_update', contentList[data.room]); 
	  console.log(rooms);
  });*/
  socket.on('disconnect', function (){
	  leaveroom(socket);
	  console.log(rooms);
  });
  socket.on('send', function (data) {
	  socket.broadcast.to(socket.custom.room).emit('recv', {peer_id:socket.custom.peer_id, data:data});
  });
});

