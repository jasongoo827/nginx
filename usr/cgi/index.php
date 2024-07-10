#!/usr/bin/php
<!DOCTYPE html>
<html>
<head>
    <title>File List</title>
</head>
<body>
    <h1>File List</h1>
    <table border="1">
        <tr>
            <th>Filename</th>
            <th>Action</th>
        </tr>
        <?php
        $dir = './usr/upload'; // 디렉토리 경로를 설정하세요.
        if ($handle = opendir($dir)) {
            while (false !== ($file = readdir($handle))) {
                if ($file != "." && $file != "..") {
                    echo "<tr>";
                    echo "<td>" . $file . "</td>";
                    echo "<td><form method='delete' action='" . $file . "'><input type='hidden'><input type='submit' value='Delete'></form></td>";
                    echo "</tr>";
                }
            }
            closedir($handle);
        }
        ?>
    </table>
</body>
</html>
