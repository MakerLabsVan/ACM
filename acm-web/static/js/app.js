var app = angular.module('ACM-Dash',[]);

app.controller('ACM-Controller', ['$scope', '$http', function($scope, $http) {
	$scope.master = {};
	$scope.raw_time = 123456;
	$scope.minutes = parseInt($scope.raw_time / 60);
	$scope.seconds = $scope.raw_time % 60;


	$scope.getTime = function() {
		$http.get("../getTime").success(function(res) {
			console.log(res);
			$scope.raw_time = res;
			$scope.minutes = parseInt(res / 60);
			$scope.seconds = res % 60

		})
	}

	$scope.resetTime = function() {
		$http.get("../resetTime").success(function(res) {
			console.log(res);
			$scope.status = "Not successful";

			if (res[0] == 1) {
				$scope.status = "Success";
				console.log($scope.status);
			}

		})
	}

}]);
