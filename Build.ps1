Param(
  [Parameter(Position=0)][string]$paramArch,
  [Parameter(Position=1)][string]$paramPathBuildDirectory
)


$OutputEncoding='utf-8'


function CreateDirectory($path)
{
  if (Test-Path $path)
  {
    return
  }

  New-Item $path -ItemType Directory
}

CreateDirectory $paramPathBuildDirectory
Set-Location $paramPathBuildDirectory

# cmake -G "Visual Studio 17 2022" -A $paramArch -D PV_DEBUG=ON -D PV_DEVELOPMENT=ON -D PV_TESTING=ON ..
cmake -G "Visual Studio 17 2022" -A $paramArch -D PV_DEBUG=OFF -D PV_DEVELOPMENT=OFF -D PV_TESTING=OFF ..
