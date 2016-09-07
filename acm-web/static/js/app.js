var app = angular.module('ACM-Dash',[]);

app.controller('ACM-Controller', ['$scope', '$http', function($scope, $http) {
	$scope.master = {};

	// initialize some dummy data for testing
	$scope.time = new laserTime(123456);

	function laserTime(raw_time) {
		this.raw_time = raw_time;
		this.set = function(newTime) { this.raw_time = newTime };

		this.hours = function() { return parseInt(this.raw_time / 3600) };
		this.minutes = function() { return parseInt(this.raw_time % 3600 / 60) };
		this.seconds = function() { return this.raw_time % 60 };

		this.charge = function() { return "hi" };
	}


	$scope.getTime = function() {
		$http.get("../getTime").success(function(res) {
			console.log("Raw Time: " + res);
			$scope.time.set(res);

		})
	}

	$scope.resetTime = function() {
		$http.get("../resetTime").success(function(res) {
			$scope.status = "Not successful";

			if (res[0] == 1) {
				$scope.status = "Success";
				console.log($scope.status);
			}

		})
	}

	$scope.registerCard = function() {
		if ($scope.id) {

			$http({
				url: "../registerCard",
				method: "POST",
				headers: { 'Content-Type': 'application/json' },
				data: JSON.stringify($scope.id)})
				.success(function(data) {
					$scope.entered = parseInt(data);
					console.log("ID sent: " + $scope.entered)
				});

			$scope.id = '';
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