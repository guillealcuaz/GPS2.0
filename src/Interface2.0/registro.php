<?php
include_once 'db.php';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $nombre = $conn->real_escape_string($_POST['nombre']);
    $correo = $conn->real_escape_string($_POST['email']);
    $contrasena = $conn->real_escape_string($_POST['password']);

    $contrasena_hash = password_hash($contrasena, PASSWORD_BCRYPT);

    $query = "INSERT INTO usuarios (nombre, correo, contrasena) VALUES (?, ?, ?)";

    if ($stmt = $conn->prepare($query)) {
        $stmt->bind_param("sss", $nombre, $correo, $contrasena_hash);
        
        if ($stmt->execute()) {
            echo "Usuario Registrado";
            header("Location: login.html");
            exit();
        } else {
            echo "Registro fallido: " . $stmt->error;
        }
        $stmt->close();
    } else {
        echo "Error en la consulta" . $conn->error;
    }
    $conn->close();
}
