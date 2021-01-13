# Install chocolatey
echo "Installing chocolatey ..."

Set-ExecutionPolicy Bypass -Scope Process -Force
[System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))

# Install dependecies
echo "Installing dependencies ..."

choco feature enable -n=allowGlobalConfirmation
choco install git
choco install cmake.install --installargs '"ADD_CMAKE_TO_PATH=System"'
choco install vswhere
choco install dotnetfx -y --ignore-package-exit-codes=3010
choco install visualstudio2019buildtools

