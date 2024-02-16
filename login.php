<?php
session_start(); 
include 'db.php';
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $correo = $conn->real_escape_string($_POST['email']);
    $contrasena = $conn->real_escape_string($_POST['password']);
    $query = "SELECT id, nombre, contrasena FROM usuarios WHERE correo = ?";

    if ($stmt = $conn->prepare($query)) {
        $stmt->bind_param("s", $correo);
        
        if ($stmt->execute()) {
            $resultado = $stmt->get_result();
            if ($resultado->num_rows === 1) {
                $usuario = $resultado->fetch_assoc();
                if (password_verify($contrasena, $usuario['contrasena'])) {
                    $_SESSION['usuario_id'] = $usuario['id'];
                    $_SESSION['usuario_nombre'] = $usuario['nombre'];
                    header("Location: index.html");
                    exit();
                } else {
                    $_SESSION['alerta'] = 'Contraseña incorrecta.';
                }
            } else {
                $_SESSION['alerta'] = 'No existe ese usuario';
            }
        } else {
            $_SESSION['alerta'] = 'Error BD' . $stmt->error;
        }    
        $stmt->close();
    } else {
        echo "Error BD" . $conn->error;
    }
    $conn->close();
}
?>
<!DOCTYPE html>
<html>
<head>
<!-- Tus estilos y scripts -->
</head>
<body>
<?php if (isset($_SESSION['alerta'])): ?>
    <div class="alert alert-info">
        <?php 
        echo htmlspecialchars($_SESSION['alerta']); 
        unset($_SESSION['alerta']); // Limpia el mensaje después de mostrarlo
        ?>
    </div>
<?php endif; ?>

<!-- Contenido de tu formulario de inicio de sesión -->

</body>
</html>