<?php
/*
* Script que se encarga de subir un fichero al servidor en formato GeoJSON.
* @author Guillermo Alcuaz
*/


session_start();
include_once 'db.php';
if (!isset($_SESSION['id_usuario'])) {
    header('Location: index.php');
    exit;
}

$uploadDir = 'misgeoJSON/';

function esGeoJSON($archivo) {
    $contenido = file_get_contents($archivo['tmp_name']);
    $data = json_decode($contenido, true);
    if ($data === null || !isset($data['type']) ||
     !in_array($data['type'], ['FeatureCollection', 'Feature', 'Point', 'LineString', 'Polygon',
      'MultiPoint', 'MultiLineString', 'MultiPolygon', 'GeometryCollection'])) {

        return false;
    }
    return true;
}

if (isset($_FILES['archivo'])) {
    if (esGeoJSON($_FILES['archivo'])) {
        $nombreArchivo = basename($_FILES['archivo']['name']);
        $nombreRutaArchivo = $uploadDir . date("j-m-Y_H-i-s_") . $nombreArchivo;
        if (move_uploaded_file($_FILES['archivo']['tmp_name'], $nombreRutaArchivo)) {
            $stmt = $conn->prepare("INSERT INTO archivos_subidos (id_usuario, archivo, ruta, fecha_subida) VALUES (?, ?, ?, NOW())");
            $stmt->bind_param("iss", $_SESSION['id_usuario'], $nombreArchivo, $nombreRutaArchivo);
            if ($stmt->execute()) {
                header('Location: uploadgeojson.php?ruta=' . ($nombreRutaArchivo));
                $stmt->close();
                exit;
            }
            $stmt->close();
        }
    }   else{

        $_SESSION['error'] = "El archivo subido no es un archivo GeoJSON válido.";
        header('Location: index.php');
        exit;
    }
    exit;
}


if (isset($_GET['ruta'])) {
    $rutaArchivo = ($_GET['ruta']);
}



$conn->close();
?>

<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset='utf-8' />
    <title>Interfaz GPS</title>
    <meta name='viewport' content='initial-scale=1,maximum-scale=1,user-scalable=no' />
    <script src='https://api.tiles.mapbox.com/mapbox-gl-js/v0.51.0/mapbox-gl.js'></script>
    <link href='https://api.tiles.mapbox.com/mapbox-gl-js/v0.51.0/mapbox-gl.css' rel='stylesheet' />
    <script src="https://code.jquery.com/jquery-3.3.1.min.js"></script> <!--Librería para descargar la imagen--> 
    <link rel="stylesheet" href="https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css"/> <!--Libreria botones-->

    <style>
        body { margin:0; padding:0; }
        #map { position:fixed; top:2.8em; bottom:0; width:100%; float:left;}
        #imagen {  display:inline-block; top:0; bottom:0;  float:right; margin-right: 1em }
        #volar {position:relative; display:inline-block; top:0; bottom:0;  float:left; margin-left: 1em }
        #home {position:relative; display:inline-block; top:0; bottom:0;  float:left; margin-left: 1em }
    </style>
</head>
<body>

<!--Botón para descargar la imagen del mapa--> 
<div id="imagen">
  <a class="btn btn-success" id="enlaceDescarga" href="" download="map.png">Descargar mapa</a> 
</div>
<!--Botón para volar a tu posición--> 
<button class="btn btn-success" id='volar'>Centrar en mi posición</button>
<!--Regresar a la página de inicio-->
<a class="btn btn-success" id="home" href="index.php">Volver a inicio</a>

<div id='map'> 
<script>
mapboxgl.accessToken = 'pk.eyJ1IjoiYWxlamFuZHJvbGFtcHJlYXZlIiwiYSI6ImNqcHFzbHBzbTB4NHc0NW9nbjJ5eDNqbmgifQ.TbEYVxeQlVtCp93MXTAZJQ';
var map = new mapboxgl.Map({
    container: 'map',
    style: 'mapbox://styles/mapbox/streets-v9',
    zoom: 16,
    center: [-3.688889, 42.352357], 
    pitch: 20,
    preserveDrawingBuffer: true,
});

/*Mofificamos el ratio de píxeles lógicos que hay por píxel físico, de forma 
*que podamos sobrepasar el máximo permitido por la API de mapbox para su
*posterior descarga en alta resolución*/
var dpi = 300;
Object.defineProperty(window, 'devicePixelRatio', {
    get: function() {return dpi / 96}
});

var src = "./misgeoJSON/" + "<?php echo htmlspecialchars(basename($rutaArchivo), ENT_QUOTES); ?>";


map.on('load', function () {

    var layers = map.getStyle().layers;

    var labelLayerId;
    for (var i = 0; i < layers.length; i++) {
        if (layers[i].type === 'symbol' && layers[i].layout['text-field']) {
            labelLayerId = layers[i].id;
            break;
        }
    }
    
     map.addSource('geojson', {
        type: "geojson",
        data: src
    });

     map.addLayer({
        'id': 'geojson-layer',
        'type': 'circle',
        source: 'geojson',
        'paint': {
            // hace los circulos mas grandes segun haces zoom, del 12 al z22
            'circle-radius': {
                'base': 1.75,
                'stops': [[12, 2], [22, 180]]
            },
            'circle-color': '#f00'
        }
    });

    map.addLayer({ //3D de los edificios
        'id': '3d-buildings',
        'source': 'composite',
        'source-layer': 'building',
        'filter': ['==', 'extrude', 'true'],
        'type': 'fill-extrusion',
        'minzoom': 15,
        'paint': {
            'fill-extrusion-color': '#aaa',

            /* Usar un 'interpolate' para conseguir una transición mas natural 
               en los edificios a medida que hacemos zoom.*/ 
            
            'fill-extrusion-height': [
                "interpolate", ["linear"], ["zoom"],
                15, 0,
                15.05, ["get", "height"]
            ],
            'fill-extrusion-base': [
                "interpolate", ["linear"], ["zoom"],
                10, 0,
                15.05, ["get", "min_height"]
            ],
            'fill-extrusion-opacity': .6
        }
    }, labelLayerId);

    // Añadir botonera de zoom y rotación del mapa. 
    map.addControl(new mapboxgl.NavigationControl()); 

    //Obtener la imagen del mapa mostrado en pantala.
    $('#enlaceDescarga').click(function() {
        var img = map.getCanvas().toDataURL('image/png')
        this.href = img
    })

    //Geolocalizar tu posición y centrar el mapa en ella.
    document.getElementById('volar').addEventListener('click', function () {
    if ('geolocation' in navigator) {
      navigator.geolocation.getCurrentPosition(function(position) {
        'use strict';
        map.flyTo({center: [position.coords.longitude,
            position.coords.latitude], zoom: 14});
      });
    }
    });
});

</script>
</div>
