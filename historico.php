<?php
include_once 'db.php';

if (isset($_SESSION['id_usuario'])) {
    $idUsuario = $_SESSION['id_usuario'];

    $consulta = $conn->prepare("SELECT ruta FROM archivos_subidos WHERE id_usuario = ?");
    $consulta->bind_param('i', $idUsuario); 
    $consulta->execute();
    $resultado = $consulta->get_result();

    echo "<ul>";
    while ($fila = $resultado->fetch_assoc()) {
        echo "<li><a href='uploadgeojson.php?ruta=" . ($fila['ruta']) . "'>" . basename($fila['ruta']) . "</a></li>";
    }
    echo "</ul>";
    $consulta->close();
} else {
    echo "Inicia sesión para ver tus archivos.";
}
?>
