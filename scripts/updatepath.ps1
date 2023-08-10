param (
    [parameter(Mandatory=$true)]
    [string]$orcaPath
)

$arrPath = [System.Environment]::GetEnvironmentVariable('PATH', 'User') -split ';'
$arrPath = $arrPath | Where-Object { $_ -ne $orcaPath } | Where-Object { $_ -ne '' }
$newPath = ($arrPath + $orcaPath) -join ';'

[System.Environment]::SetEnvironmentVariable('PATH', $newPath, 'User')
# echo $newPath
