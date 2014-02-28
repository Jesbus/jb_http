<html><body>
<?php
print_r($_SERVER['argv']);
parse_str($_SERVER['argv'][1], $_GET);
parse_str($_SERVER['argv'][2], $_POST);
print_r($_GET);
print_r($_POST);
for ($i=0;$i<10;$i++)
{
	echo "$i<br/>";
}
?>
<form method="POST">
<input type="text" name="goto" value="344"/>
<input type="text" name="p2" value="wer"/>
<input type="submit" value="done"/><!--
<img src="venn_getallen.png"/>-->
</body></html>
