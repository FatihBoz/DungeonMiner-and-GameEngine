Add-Type -AssemblyName System.Drawing
$i = [System.Drawing.Bitmap]::FromFile("c:\Users\fatih\Desktop\DungeonMiner\DungeonTileset.bmp")
Write-Output ("W:" + $i.Width)
Write-Output ("H:" + $i.Height)
$i.Dispose()
