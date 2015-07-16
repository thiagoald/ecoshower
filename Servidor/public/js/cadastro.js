$("#cadastro-button").click(function(event){
		 event.preventDefault();
	if( validacao() == true)
	{
		$('button').fadeOut(0);
		$('form').fadeOut(500).promise().done(function(){window.location.href = 'login';});
	 	$('.wrapper').addClass('form-success').promise().done();
	 	$("#feedbackMessage")[0].innerHTML = "Cadastro com sucesso!";
	}
});

function validacao(){

	if($("#usuario")[0].value == "" && $("#senha")[0].value == ""){
		$("#feedbackMessage")[0].innerHTML = "Por favor, preencha Usuário e Senha.";
		$("#usuario")[0].focus();
		return false;
	}
	else if($("#usuario")[0].value == ""){
		$("#feedbackMessage")[0].innerHTML = "Por favor, preencha o campo Usuário.";
		$("#usuario")[0].focus();
		return false;
	}
	else if($("#senha")[0].value == "")
	{
		$("#feedbackMessage")[0].innerHTML = "Por favor, preencha o campo Senha.";
		$("#senha")[0].focus();
		return false;
	}
	else
	{
		return true;
	}
 
}