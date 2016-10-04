var app = angular.module('ACM-Dash',[]);

app.controller('ACM-Controller', ['$scope', '$http', function($scope, $http) {
	$scope.master = {};

	$scope.time = new laserTime(123456);
	function laserTime(rawTime) {
		this.rawTime = rawTime;
		this.set = function(newTime) { this.rawTime = newTime };

		this.hours = function() { return parseInt(this.rawTime / 3600) };
		this.minutes = function() { return parseInt(this.rawTime % 3600 / 60) };
		this.seconds = function() { return this.rawTime % 60 };

		this.charge = function() { return (1.5 * parseFloat(this.rawTime / 60)).toFixed(2) };
	}


	$scope.getTime = function() {
		$http.get("../getTime").success(function(res) {
			if (res == 2) {
				console.log("Error");
				$scope.time.set("Error");
			}
			else {
				console.log("Raw Time: " + res);
				$scope.time.set(res);
			}
			

		})
	}

	$scope.resetTime = function() {
		$http.get("../resetTime").success(function(res) {
			$scope.status = "Not successful";

			if (res[0] == 1) {
				$scope.status = "Success";
			}

			console.log($scope.status);

		})
	}

	$scope.user = {
		uid: "",
		memberName: "",
		memberType: "0",
		startDay: "",
		laserA: "0",
		laserB: "",
		shopbot: "",
		wood: "",
		metal: "",
		textile: "",
		threeD: ""
	};

	$scope.registerCard = function() {

		console.log(JSON.stringify($scope.user));

		if ($scope.user.uid && $scope.user.memberName) {
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
			$scope.user.memberType = "1";
			$scope.user.laserA = "0";
		}
	}

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