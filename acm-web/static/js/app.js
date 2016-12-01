var app = angular.module('ACM-Dash',[]);

app.config(function($interpolateProvider) {
	$interpolateProvider.startSymbol('//');
    $interpolateProvider.endSymbol('//');
});

app.controller('ACM-Controller', ['$scope', '$http', '$interpolate', function($scope, $http, $interpolate) {
	$scope.master = {};
	$scope.tab = [ "active", "", "" ];
	$scope.tabPane = [ "tab-pane active", "tab-pane", "tab-pane"];
	$scope.progress = { isWaiting: false };
	$scope.user = {
		isNew: true,
		uid: "",
		memberName: "",
		memberType: "0",
		startDay: "",
		laserA: "0",
		laserB: "1",
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
		$scope.tab = [ "", "", ""];
		$scope.tabPane = [ "tab-pane", "tab-pane", "tab-pane" ];
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
				//$scope.getTime();
			}
			else {
				$scope.isError = true;
			}
			$scope.progress.isWaiting = false;
		});
	};

	$scope.registerCard = function(isNew) {
		$scope.user.isNew = isNew;
			
		if ($scope.user.uid && $scope.user.memberName) {
			console.log(JSON.stringify($scope.user));
			var input = $scope.user.uid;

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
	}

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
		
		$scope.$apply();
    });
	socket.on('refresh', function(msg) {
		$scope.refresh = msg;
		$scope.$apply();
	})

	/*$scope.drawDonut = function() {
		var vis = d3.select("#time-display").append("g").append("svg");
		var pi = Math.PI;

		var containerWidth = d3.select("#content").node().getBoundingClientRect().width;
		var containerHeight = d3.select("#content").node().getBoundingClientRect().height;
		var j = 0.25;
		var x = j * containerWidth;
		var y = containerHeight/2;
		console.log(y);

		vis.attr("width", containerWidth).attr("height", 200);

		var arc = d3.arc()
			.innerRadius(75)
			.outerRadius(100)
			.startAngle(0)
			.endAngle(2 * pi);

		for (i = 0; i < 3; i++) {
			vis.append("path")
				.attr("d", arc)
				.attr("fill", "grey")
				.attr("transform", "translate(" + x + "," + 100 + ")");
				j += 0.25;
				x = j * containerWidth;
		};
		// ---------------------------------------------------------------
		j = 0.25;
		x = j * containerWidth;

		var data = [ $scope.hours, $scope.minutes, $scope.seconds ];
		var radianScale = d3.scaleLinear().domain([0, 59]).range([0, 2*pi]);
		data[0] = radianScale($scope.hours);
		data[1] = radianScale($scope.minutes);
		data[2] = radianScale($scope.seconds);

		var arc2 = d3.arc()
			.innerRadius(75)
			.outerRadius(100)
			.startAngle(0);

		var arc3 = d3.arc()
			.innerRadius(75)
			.outerRadius(100)
			.startAngle(0)
			.endAngle(pi/180);


		for (i = 0; i < 3; i++) {
			arc2.endAngle(data[i]);
			vis.append("path")
				.attr("fill", "blue")
				.attr("transform", "translate(" + x + "," + 100 + ")")
				.attr('d', arc3)
				.transition().delay(250)
				.attr("d", arc2);
				j += 0.25;
				x = j * containerWidth;
		};

	};*/

}]);