$(function() {

    $('#sidebar li').click(function(e) {

    	// Highlight selected tab
    	$clickedElement = $(this);
    	$clickedElement.parent().find('li').removeClass('active');
    	$clickedElement.addClass('active');

    	// Switch to tab pane
    	var href = $clickedElement.children().attr('href');
    	$(href).parent().find('div').removeClass('active');
    	$(href).addClass('active');

    });

});

$(document).ready(function() {
    var socket = io.connect('http://localhost:5000');
    console.log("Socket connected.");

    socket.on('scan', function(msg) {
        console.log("hi from arduino: " + msg.data);
    });

});