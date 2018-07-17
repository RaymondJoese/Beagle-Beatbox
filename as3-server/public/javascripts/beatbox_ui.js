"use strict";
// Client-side interactions with the browser.

// Make connection to server when web page is fully loaded.
var UPDATE_VOL_PBM_MODE = 300;
var MS_UPDATE_TIME = 1000;
var TIME_HIDE_ERR_BOX = 10000;
var SECS_PER_MIN = 60;
var HOURS_PER_DAY = 24;

// var nodeJS_Working = true;
// var c_app_Working = true;

var nodeErr;
var appErr;

var socket = io.connect();
$(document).ready(function() {

	$('#modeOff').click(function(){
		sendPrimeCommand("mode off");
	});
	$('#modeRock').click(function(){
		sendPrimeCommand("mode rock");
	});
	$('#modeOther').click(function(){
		sendPrimeCommand("mode other");
	});
	$('#volumeDown').click(function(){
		sendPrimeCommand("vol down");
	});
	$('#volumeUp').click(function(){
		sendPrimeCommand("vol up");
	});
	$('#bpmDown').click(function(){
		sendPrimeCommand("bpm down");
	});
	$('#bomUp').click(function(){
		sendPrimeCommand("bpm up");
	});

	$('#bass').click(function(){
		sendPrimeCommand("play 1");
	});
	$('#hihat').click(function(){
		sendPrimeCommand("play 2");
	});
	$('#snare').click(function(){
		sendPrimeCommand("play 3");
	});
	$('#tomHi').click(function(){
		sendPrimeCommand("play 4");
	});
	$('#tomMid').click(function(){
		sendPrimeCommand("play 5");
	});
	$('#tomLow').click(function(){
		sendPrimeCommand("play 6");
	});
	$('#splash').click(function(){
		sendPrimeCommand("play 7");
	});

	window.setInterval(function(){
		sendPrimeCommand("get all");
	}, UPDATE_VOL_PBM_MODE);

	socket.on('all', function(str) { // output ALL: 'vol' 'bpm' 'mode' 1,2,3 are index!!!!!
		var result = str.split(" ");
		document.getElementById("volumeid").value = result[1];
		document.getElementById("bpmId").value = result[2];
		document.getElementById("modeid").innerHTML = result[3];
		clearTimeout(appErr);
	});

	window.setInterval(function() {
		sendRequest('uptime');
	}, MS_UPDATE_TIME);

	socket.on('fileContents', function(result) {
		var fileName = result.fileName;
		var contents = result.contents.split(' ');

		var domObj;
		switch(fileName) {
			case 'uptime':
				domObj = $('#uptimeId');
				break;
			default:
				console.log("Unknown DOM object: " + fileName);
				return;
		}
		// Make linefeeds into <br> tag.
		// convert uptime to Hour:Mins:Secs
		contents = "Device up for:\n" + (parseInt(((parseInt(contents[0])/SECS_PER_MIN)/SECS_PER_MIN)%HOURS_PER_DAY)<10?'0':'') + parseInt(((parseInt(contents[0])/SECS_PER_MIN)/SECS_PER_MIN)%HOURS_PER_DAY) + ':'
			+ (parseInt((parseInt(contents[0])/SECS_PER_MIN)%SECS_PER_MIN) < 10?'0':'') + parseInt((parseInt(contents[0])/SECS_PER_MIN)%SECS_PER_MIN) + ':'
			+ (parseInt(parseInt(contents[0])%SECS_PER_MIN)<10?'0':'') + parseInt(parseInt(contents[0])%SECS_PER_MIN) + '(H:M:S)';

		contents = replaceAll(contents, "\n", "<br/>");
		domObj.html(contents);
	});



	window.setInterval(function () {
		nodeErr = setTimeout (function () {
			document.getElementById("error-box").style.display = "block";
			document.getElementById("error-text").innerHTML = "SERVER ERROR: NodeJS server is no longer running.";
		}, MS_UPDATE_TIME);
		appErr = setTimeout (function() {
			if (document.getElementById("error-box").style.display === "none") {
				document.getElementById("error-box").style.display = "block";
				document.getElementById("error-text").innerHTML = "SERVER ERROR: No response from beat-box application";
			}
		}, MS_UPDATE_TIME);
	}, MS_UPDATE_TIME);

	socket.on('nodeJsGood', function(data) {
		clearTimeout(nodeErr);
	});

	socket.on('appGood', function(data) {
		clearTimeout(appErr);
	});

	window.setInterval(function() {
		document.getElementById("error-box").style.display = "none";
	}, TIME_HIDE_ERR_BOX);

});

function sendPrimeCommand(message) {
	socket.emit('prime', message);
};

function updateTime() {
	var date = new Date();
	var hours = (date.getHours()<10?'0':'') + date.getHours();
	var mins = (date.getMinutes()<10?'0':'') + date.getMinutes();
	var secs = (date.getSeconds()<10?'0':'') + date.getSeconds();
	var timeStr = (date.getHours()<10?'0':'') + date.getHours() + ':'
				+ (date.getMinutes()<10?'0':'') + date.getMinutes() + ':'
				+ (date.getSeconds()<10?'0':'') + date.getSeconds();
	// var timeStr = date.getHours() + ':'
	// 			+ date.getMinutes() + ':'
	// 			+ date.getSeconds();
	// $('#status').html("Its now<br/>" + timeStr);
	document.getElementById("status").innerHTML = timeStr;
}

function sendRequest(file) {
	console.log("Requesting '" + file + "'");
	socket.emit('proc', file);
}

function replaceAll(str, find, replace) {
	return str.replace(new RegExp(find, 'g'), replace);
}
