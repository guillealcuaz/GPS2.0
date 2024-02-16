<?php

// Esto NO es una práctica recomendada,
// NO se deben subir credenciales a un repositorio
// Al no contener información confidencial y al ser un entorno de prueba se hace como caso excepcional

$servername = "localhost";
$username = "master";
$password = "mastertfg123";
$dbname = "gpslogin";

$conn = new mysqli($servername, $username, $password, $dbname);
if ($conn->connect_error) {
    die("Error de conexión: " . $conn->connect_error);
}
$conn->set_charset("utf8");
