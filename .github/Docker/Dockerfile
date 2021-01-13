# escape=`
# from https://docs.microsoft.com/fr-fr/visualstudio/install/build-tools-container?view=vs-2019
# Use the latest Windows Server Core image with .NET Framework 4.8.
#FROM mcr.microsoft.com/windows/servercore:ltsc2019 AS cmake_win_base
FROM mcr.microsoft.com/dotnet/framework/runtime:4.7.2-windowsservercore-ltsc2019 AS cmake_win_base

# Restore the default Windows shell for correct batch processing.
SHELL ["cmd", "/S", "/C"]

ENV TempDir=C:\TEMP
ENV InstallationDir=C:\TEMP\Installation
RUN powershell -Command mkdir $env:InstallationDir

# Download the Build Tools bootstrapper.
RUN powershell -Command Invoke-WebRequest -Uri "https://aka.ms/vs/16/release/vs_buildtools.exe" -OutFile $env:TempDir\vs_buildtools.exe
#ADD https://aka.ms/vs/16/release/vs_buildtools.exe C:\TEMP\vs_buildtools.exe

# Install Build Tools with the Microsoft.VisualStudio.Workload.AzureBuildTools workload, excluding workloads and components with known issues.
RUN C:\TEMP\vs_buildtools.exe --quiet --wait --norestart --nocache `
    --installPath $env:TempDir\BuildTools `
    --add Microsoft.VisualStudio.Workload.AzureBuildTools `
    --add Microsoft.VisualStudio.Component.Windows10SDK.18362 `
    --remove Microsoft.VisualStudio.Component.Windows10SDK.10240 `
    --remove Microsoft.VisualStudio.Component.Windows10SDK.10586 `
    --remove Microsoft.VisualStudio.Component.Windows10SDK.14393 `
    --remove Microsoft.VisualStudio.Component.Windows81SDK `
 || IF "%ERRORLEVEL%"=="3010" EXIT 0

# from https://stackoverflow.com/questions/60288250/restart-docker-container-during-build-process
RUN powershell -NoProfile -ExecutionPolicy Bypass -Command `
    Invoke-WebRequest "https://aka.ms/vs/16/release/vs_community.exe" `
    -OutFile "%TEMP%\vs_community.exe" -UseBasicParsing

RUN "%TEMP%\vs_community.exe"  --quiet --wait --norestart --noUpdateInstaller `
    --add Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    --add Microsoft.VisualStudio.Component.Windows10SDK.18362

# Define the entry point for the docker container.
# This entry point starts the developer command prompt and launches the PowerShell shell.
ENTRYPOINT ["C:\\TEMP\\BuildTools\\Common7\\Tools\\VsDevCmd.bat", "&&", "powershell.exe", "-NoLogo", "-ExecutionPolicy", "Bypass"]

SHELL ["powershell", "-NoLogo", "-ExecutionPolicy", "Bypass"]

COPY ./install_depends.ps1 C:\TEMP\Installation
#RUN powershell -File C:\TEMP\Installation\install_depends.ps1

RUN echo "Installing dependencies"
RUN Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))

RUN choco feature enable -n=allowGlobalConfirmation
RUN choco install git
RUN choco install cmake.install --installargs '"ADD_CMAKE_TO_PATH=System"'
#RUN choco install -y --ignore-package-exit-codes=3010 dotnetfx -Restart:$true

FROM cmake_win_base as cmake_win_base_rebooted

#RUN choco install visualstudio2019buildtools
#RUN choco install visualstudio2019professional
RUN Invoke-WebRequest -Uri "https://dist.nuget.org/win-x86-commandline/v5.7.0/nuget.exe" -OutFile $env:TempDir\nuget.exe

RUN C:\TEMP\nuget.exe install vswhere -OutputDirectory $env:InstallationDir
RUN [Environment]::SetEnvironmentVariable('"Path"', $env:Path + '";C:\TEMP;"' + $env:InstallationDir + '\vswhere.2.8.4\tools', [EnvironmentVariableTarget]::Machine)
RUN echo $env:Path
RUN ls $env:InstallationDir\vswhere.2.8.4\tools
RUN vswhere

WORKDIR ${TempDir}
RUN git clone "https://github.com/google/googletest.git"

WORKDIR ${TempDir}/googletest
RUN git pull --tags
RUN git checkout tags/release-1.10.0
RUN mkdir build

WORKDIR ${TempDir}/googletest/build
RUN cmake --version
RUN cmake ../ -G '"Visual Studio 16 2019"'
#RUN cmake ../
RUN msbuild googletest.sln

