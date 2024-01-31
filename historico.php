<?php
$misgeoJSON = "misgeoJSON/";
$subidos = scandir($misgeoJSON);

echo "<ul>";
foreach ($subidos as $archivo) {
    if ($archivo != "." && $archivo != "..") {
        echo "<li><a href='" . $misgeoJSON . $archivo . "'>" . $archivo . "</a></li>";
    }
}
echo "</ul>";
?>
