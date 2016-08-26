var app = angular.module('ACM-Dash',[]);

app.controller('ACM-Controller', ['$scope', '$http', function($scope, $http) {
	$scope.master = {};
	$scope.time = 0;

	$scope.getTime = function() {
		$http.get("../on").success(function(res) {
		console.log(res[0]);


	})}

}]);
