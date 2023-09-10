param (
    [parameter(Mandatory=$true)]
    [string]$orcaPath,

    [switch]$remove
)

$arrPath = [System.Environment]::GetEnvironmentVariable('PATH', 'User') -split ';'
$arrPath = $arrPath | Where-Object { $_ -ne $orcaPath } | Where-Object { $_ -ne '' }
if (-not $remove) {
    $arrPath += $orcaPath
}

$newPath = $arrPath -join ';'

[System.Environment]::SetEnvironmentVariable('PATH', $newPath, 'User')
