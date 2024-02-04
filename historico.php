<?php
include 'db.php';

if (isset($_SESSION['id_usuario'])) {
    $idUsuario = $_SESSION['id_usuario'];

    $consulta = $conn->prepare("SELECT archivo FROM archivos_subidos WHERE id_usuario = ?");
    $consulta->bind_param('i', $idUsuario); 
    $consulta->execute();
    $resultado = $consulta->get_result();
    $carpeta = 'misgeoJSON/';

    echo "<ul>";
    while ($fila = $resultado->fetch_assoc()) {
        $ruta = $carpeta . $fila['archivo'];
        echo "<li><a href='" . $ruta . "'>" . $fila['archivo'] . "</a></li>"; 
    }
    echo "</ul>";
    $consulta->close();
} else {
    echo "Inicia sesión para ver tus archivos.";
}
?>
