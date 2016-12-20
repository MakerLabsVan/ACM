var app = angular.module('ACM-Dash',[]);

// Changing interpolation symbols here
// Flask Jinga2 templating uses {{}}, which
// inteferes with Angular. 
app.config(function($interpolateProvider) {
	$interpolateProvider.startSymbol('//');
    $interpolateProvider.endSymbol('//');
});

app.controller('ACM-Controller', ['$scope', '$http', '$interpolate', '$interval', function($scope, $http, $interpolate, $interval) {
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
	var PATH = "../static/img/users/";	
	$scope.display = {
			id: "",
			name: "",
			image: PATH + "GUEST.jpg",
			startDay: "",
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

	$scope.time = new laserTime(123456);
	function laserTime(rawTime) {
		this.rawTime = rawTime;
		this.set = function(newTime) { this.rawTime = newTime };

		this.hours = function() { return parseInt(this.rawTime / 3600) };
		this.minutes = function() { return parseInt(this.rawTime % 3600 / 60) };
		this.seconds = function() { return this.rawTime % 60 };

		this.charge = function() { return (1.5 * parseFloat(this.rawTime / 60)).toFixed(2) };
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
			if (res[0] == 1) {
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
			console.log(JSON.stringify($scope.user));
			var input = $scope.user.uid;
			$scope.user.isNew = isNew;
			
			$http({
				url: "../registerCard",
				method: "POST",
				headers: { 'Content-Type': 'application/json' },
				data: JSON.stringify($scope.user)})
				.success(function(data) {
					
					if (data != input) {
						$scope.entered = "Error";
						console.log($scope.entered);
					}
					else {
						$scope.entered = "User " + data + " registered.";
						console.log("ID registered: " + parseInt(data));				
					}

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
	}, 60*60*1000);

	// socket for pushing data from server to web page
	var socket = io.connect("http://localhost:5000");
	console.log("Socket Connected.");
	socket.on('scan', function(msg) {
		$scope.activeTab(0);
		$scope.display.id = msg;
		$scope.display.name = "";
		$scope.display.image = PATH + "GUEST.jpg";
		$scope.refresh = "";
		$scope.$apply();
	});
	socket.on('data', function(msg) {
		$scope.activeTab(0);
		if (msg) {
			$scope.display.id = msg[1];
			$scope.display.name = msg[2];
			$scope.display.image = PATH + msg[2] + ".jpg";
			$scope.display.startDay = msg[4];
			$scope.display.studio = msg[8];

			for (var i = 0; i < $scope.display.access.length; i++) {
				$scope.display.access[i] = parseInt(msg[i + 9]) ? "green" : "red";
			}

			$scope.display.lifeTime = (parseInt(msg[16]) / 60).toFixed(0);
			$scope.display.monthTime = (parseInt(msg[17]) / 60).toFixed(0);	

			$scope.display.lifeTime = $scope.display.lifeTime.toString() + " minutes"
			$scope.display.monthTime = $scope.display.monthTime.toString() + " minutes"
		}
		else {
			$scope.refresh = "User does not exist and/or is no longer a member";
		}
		
		$scope.$apply();
    });
	socket.on('refresh', function(msg) {
		$scope.refresh = msg;
		$scope.$apply();
	});

}]);