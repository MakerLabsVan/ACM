$(function(){

    $('#sidebar li').click(function(e) {

      $clickedElement = $(this);

      $clickedElement.parent().find('li').removeClass('active');
      $clickedElement.addClass('active');
    });
})