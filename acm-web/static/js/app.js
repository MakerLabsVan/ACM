var app = angular.module('ACM-Dash',[]);

// Changing interpolation symbols here
// Flask Jinga2 templating uses {{}}, which
// inteferes with Angular. 
app.config(function($interpolateProvider) {
	$interpolateProvider.startSymbol('//');
    $interpolateProvider.endSymbol('//');
});

app.controller('ACM-Controller', ['$scope', '$http', '$interpolate', '$interval', function($scope, $http, $interpolate, $interval) {
	var PATH = "https://s3-us-west-2.amazonaws.com/makerlabs.acm/user_images/";
	var DROP_IN_RATE = 1.5;
	var SECONDS_IN_MINUTE = 60;		
	var SECONDS_IN_HOUR = 3600;
	var MILLISECONDS_PER_HOUR = SECONDS_IN_HOUR*1000;	
	
	$scope.master = {};
	// For tab switching
	$scope.tab = [ "active", "", "", "" ];
	$scope.tabPane = [ "tab-pane active", "tab-pane", "tab-pane", "tab-pane"];
	// Progress bar for guest card screen
	$scope.progress = { isWaiting: false };
	// Model for registration form and user data table
	$scope.user = {
		isNew: true,
		uid: "",
		memberName: "",
		memberType: "0",
		startDay: "",
		laserA: "0",
		laserB: "0",
		shopbot: "0",
		wood: "0",
		metal: "0",
		textile: "0",
		threeD: "0"
	};
	$scope.display = {
			id: "",
			name: "",
			image: PATH + "guest.jpg",
			type: "",
			startDay: "",
			axis: "",
			studio: "",
			access: ["red", "red", "red", "red", "red", "red", "red"],
			lifeTime: "",
			monthTime: ""
	};

	$scope.activeTab = function(clicked) {
		// reset then add active class to clicked tab
		$scope.tab = [ "", "", "", "" ];
		$scope.tabPane = [ "tab-pane", "tab-pane", "tab-pane", "tab-pane" ];
		$scope.tab[clicked] = "active";
		$scope.tabPane[clicked] += " " + $scope.tab[clicked];
	};

	$scope.time = new laserTime(0);
	
	function laserTime(rawTime) {
		this.rawTime = rawTime;
		this.set = function(newTime) { this.rawTime = newTime };

		this.hours = function() { return parseInt(this.rawTime / SECONDS_IN_HOUR) };
		this.minutes = function() { return parseInt(this.rawTime % SECONDS_IN_HOUR / SECONDS_IN_MINUTE) };
		this.seconds = function() { return this.rawTime % SECONDS_IN_MINUTE };

		this.charge = function() { return (DROP_IN_RATE * parseFloat(this.rawTime / SECONDS_IN_MINUTE)).toFixed(2) };
	};

	// Initialize the summary tables
	$http.get("../laserData/A").success(function(res) {
			$scope.laserAData = res;				
	});
	$http.get("../laserData/B").success(function(res) {
		$scope.laserBData = res;
	});

	$scope.getTime = function() {
		$scope.progress.isWaiting = true;
		$http.get("../getTime").success(function(res) {
			if (res == 2) {
				$scope.isError = true;		
				$scope.time.set(0);	
			}
			else {
				console.log("Raw Time: " + res);
				$scope.isError = false;	
				$scope.time.set(res);			
			}
			$scope.progress.isWaiting = false;			
		});
	};

	$scope.resetTime = function() {
		$scope.progress.isWaiting = true;
		$http.get("../resetTime").success(function(res) {
			if (res == 1) {
				$scope.isError = false;
				$scope.time.set(0);
			}
			else {
				$scope.isError = true;
			}
			$scope.progress.isWaiting = false;
		});
	};

	$scope.registerCard = function(isNew) {			
		if ($scope.user.uid && $scope.user.memberName && $scope.user.startDay) {
			var input = $scope.user.uid;
			$scope.user.isNew = isNew;
			$scope.progress.isWaiting = true;
			$scope.entered = "Registering...";
			
			$http({
				url: "../registerCard",
				method: "POST",
				headers: { 'Content-Type': 'application/json' },
				data: JSON.stringify($scope.user)})
				.success(function(data) {
					if (data == 0) {
						$scope.entered = "Error with RFID registration. Try again.";
					}
					else if (data != input) {
						$scope.entered = "Error with database registration. Try again.";
					}
					else {
						$scope.entered = "User " + data + " registered.";				
					}

					$scope.progress.isWaiting = false;
				});

			$scope.user.uid = "";
			$scope.user.memberName = "";
			$scope.user.memberType = "0";
		}
		else {
			$scope.entered = "All fields are required."
		}
	}

	// Refresh summary every hour
	$interval(function() {
		console.log("Refreshing Laser Data");
		$http.get("../laserData/A").success(function(res) {
			$scope.laserAData = res;				
		})
		$http.get("../laserData/B").success(function(res) {
			$scope.laserBData = res;
		})
	}, MILLISECONDS_PER_HOUR);

	// socket for pushing data from server to web page
	var socket = io.connect("http://localhost:5000/");
	socket.on('scan', function(msg) {
		$scope.activeTab(0);
		$scope.display.id = msg;
		$scope.display.name = "";
		$scope.display.image = PATH + "guest.jpg";
		$scope.refresh = "";
		$scope.$apply();
	});
	socket.on('data', function(msg) {
		$scope.activeTab(0);
		if (msg) {
			$scope.display.id = msg[1];
			$scope.display.name = msg[2];
			$scope.display.image = PATH + msg[1] + ".jpg"; //images are named by the user's id number. Must end in .jpg, cannot be .JPG

			if (msg[3] == 0) {
				$scope.display.type = "Member";
			}
			else if (msg[3] == 1) {
				$scope.display.type = "Staff";
			}
			else {
				$scope.display.type = "n/a"
			}

			$scope.display.startDay = msg[4];
			$scope.display.axis = parseInt(msg[6]) ? "Yes" : "No";
			$scope.display.studio = (msg[8] == "") ? "n/a" : msg[8];

			for (var i = 0; i < $scope.display.access.length; i++) {
				$scope.display.access[i] = parseInt(msg[i + 9]) ? "green" : "red";
			}

			$scope.display.lifeTime = (parseInt(msg[16]) / SECONDS_IN_MINUTE).toFixed(0).toString() + " minutes";
			$scope.display.monthTime = (parseInt(msg[17]) / SECONDS_IN_MINUTE).toFixed(0).toString() + " minutes";	

		}
		else {
			$scope.refresh = "User does not exist and/or is no longer a member";
		}
		
		$scope.$apply();
    });

}]);