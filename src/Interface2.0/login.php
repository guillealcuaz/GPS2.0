<?php
session_start();
include_once 'db.php';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $correo = $conn->real_escape_string($_POST['correo']);
    $contrasena = $conn->real_escape_string($_POST['contrasena']);
    
    $query = "SELECT id, nombre, contrasena FROM usuarios WHERE correo = ?";
    if ($stmt = $conn->prepare($query)) {
        $stmt->bind_param("s", $correo);
        if ($stmt->execute()) {
            $resultado = $stmt->get_result();
            if ($resultado->num_rows === 1) {
                $usuario = $resultado->fetch_assoc();
                if (password_verify($contrasena, $usuario['contrasena'])) {
                    $_SESSION['id_usuario'] = $usuario['id'];
                    $_SESSION['nombre_usuario'] = $usuario['nombre'];
                    header("Location: index.php");
                    exit();
                }
            }
        }
        $stmt->close();
    }
    $conn->close();
    header("Location: login.html");
    exit();
}