<?php
	if ( array_key_exists ( 'message', $_POST ) )
	{
		$to = "you@example.com";
		
		$from_header = "From: you@example.com";

		$contents = "Router Was Rebooted " . $_POST['message'] . " Times.";

		$subject = "Router Was Rebooted";

		mail($to, $subject, $contents, $from_header);
	}

?>