var app = angular.module("synthctrl",[]);
var fileListCtrl = null;
app.controller('FileList', function($scope){
	$scope.files = ["hoge.json","fuga.json", "poko.json"];	
	console.log($scope.files);
	
	$scope.load = function(index){
		var fn = $scope.files[index];
		man.RPC("load", fn, function(res){
			$("#file_name").val(res.name);
			$("#score_code").val(res.content);	
		});
	}
	fileListCtrl = $scope;
});

var man = null;

function save_online()
{
	man.RPC("save", {name:$("#file_name").val(), content:$("#score_code").val()}, function(res){
		console.log(res);	
	});
}

function remove_ctrl()
{
	if(!window.confirm("Do you really want to remove " + $("#file_name").val() + " ?")){
		return;
	}
	man.RPC("remove", {name:$("#file_name").val()}, function(res){
		console.log(res);
		loadlist();	
	});
}

function run()
{
	man.RPC("deploy", {name:$("#file_name").val()}, function(res){
		console.log(res);
	});
    dashboard();
}

function dashboard()
{
    $("#dashboard").css({"display":"block"});
    /*{"position":"absolute","top":"0px","left":"0px","width":"100%","height":"100%","background-color":"gray"});
    $("#gpio0btn").css({"width":"50%","height":"80%"});
    $("#gpio1btn").css({"width":"50%","height":"80%"});
    */
}

function closedashboard()
{
    $("#dashboard").css({"display":"none"});
}

function OnGPIO0()
{
	man.RPC("gpio0", {}, function(res){
		console.log(res);	
	});
}

function OnGPIO1()
{
	man.RPC("gpio1", {}, function(res){
		console.log(res);	
	});
}

function RPCManager(wsurl)
{
	this.srv_socket = io.connect(wsurl);
	this.SocketIOEvt = {};
	this.RPCS = {};
	this.RPCID = 0;	
}
RPCManager.prototype.RPC = function(evt, data, callback, options){
	var me = this;
	if(!('RPC' in me.SocketIOEvt)){
		me.srv_socket.on('RPC', function(res){
			me.RPCS[res.RPCID](res.data);
			delete me.RPCS[res.RPCID];
		});
		me.SocketIOEvt['RPC'] = true;
	}
	me.RPCS[me.RPCID] = callback;
	var req = {RPCID:me.RPCID, evt:evt, data:data};
	me.srv_socket.emit('RPC', req);
	me.RPCID++;
};

/**
 * Broadcasting based RPC. (Utility)
 */
RPCManager.prototype.on = function(evt, callback, options)
{
	var me = this;
	if(!(evt in me.SocketIOEvt)){
		me.srv_socket.on(evt, callback);
		me.SocketIOEvt[evt] = true;
	}
};

function loadlist()
{
	man.RPC('list', {}, function(res){
		fileListCtrl.files = res["files"];
		fileListCtrl.$apply();		
	});	
}

var cnt = 0;
$("document").ready(function(){
	man = new RPCManager("192.168.11.7:8081");
	loadlist();

    var touchsupport = ('ontouchstart' in window);
    var bindevent = touchsupport ? "touchstart" : "click";
    $("#gpio0btn").on(bindevent, function(event){
        OnGPIO0();
        $("#gpio0btn").html(""+(++cnt));
        event.preventDefault();
    });

    $("#gpio1btn").on(bindevent, function(event){
        OnGPIO1();
        $("#gpio1btn").html(""+(++cnt));
        event.preventDefault();
    });
    $("#closebtn").on(bindevent, function(event){
        closedashboard(); 
        event.preventDefault(); 
    });

});
