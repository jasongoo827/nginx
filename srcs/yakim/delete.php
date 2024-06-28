#!/usr/bin/php
<?php
$status=unlink('data.txt');
if($status)
{
	echo "File deleted successfully";
}
else
{
	echo "Sorry!";
}
?>