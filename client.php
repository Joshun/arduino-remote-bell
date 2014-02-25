<!DOCTYPE html>
<html>
<head>
<title>Solenoid Bell (response)</title>
</head>

<body>
<?php

define("INPUT_MAX", "255");
define("INPUT_MIN", "0");
define("FIFO_PATH", "/tmp/my_fifo.in");
define("FIFO_OUT_PATH", "/tmp/my_fifo.out");
define("FIFO_MODE", "w");
define("FIFO_OUT_MODE", "r");
define("MAX_FEEDBACK", "20");

$EOT = chr(4);

function send_data($data_str)
{
	$pipe_file = fopen(FIFO_PATH, FIFO_MODE);
	fwrite($pipe_file, $data_str, strlen($data_str));
	fclose($pipe_file);
	printf("Sent %s to server.", $data_str);
}

function receive_data()
{
	$pipe_file = fopen(FIFO_OUT_PATH, FIFO_OUT_MODE);
	$data_str = stream_get_contents($pipe_file);
	return $data_str;
}

$arduino_str = $_GET['control'];
$from_str = $_GET['who'];
$message_str = $_GET['message'];

send_data($arduino_str . $EOT . $from_str . $EOT . $message_str);
$received_str = receive_data();

printf("<br>Response from Arduino: %s", $received_str);
	
echo '<br> <br> <a href="index.html">Return</a>';

?>
</body>

</html>
