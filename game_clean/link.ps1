<#
	.SYNOPSIS
		symlink files from a retail Team Fortress 2 build to a patched build
#>

param (
	[Parameter(Mandatory=$true)]
	[string]$TF2Dir
)

$VerbosePreference = "Continue"
$OutPath = "$PSScriptRoot\..\game"
$GameClean = "$PSScriptRoot\."


function Make-Symlink {
	param (
		[String]$Path
	)

	$linkpath = "$OutPath/$Path"
	$targetpath = "$TF2Dir/$Path"

	Write-Verbose -Message "Linking $linkpath to $targetpath"
	New-Item -ItemType SymbolicLink -Path $linkpath -Target $targetpath
}

function Glob-Symlink {
	param (
		[String]$Path,
		[String]$Glob
	)

	Get-ChildItem -Path "$TF2Dir\$Path\*" -Include $Glob | % { $_.Name } | % { Make-Symlink -Path $Path/$_ }
}

New-Item -ItemType Directory -Path $OutPath

Write-Verbose -Message "Copying $GameClean/copy/ to $OutPath"
Copy-Item -Recurse -Force $GameClean/clean/* $OutPath

Write-Verbose -Message "Creating $OutPath/tf/materials"
New-Item -Type Directory -Path $OutPath/tf/materials

$targets = "hl2","platform"
$targets += ,"maps","media","resource","scripts" | % { "tf/$_" }
$targets += ,"models","vgui" | % { "tf/materials/$_" }
ForEach ($t in $targets) {
	Make-Symlink -Path $t
}

Glob-Symlink -Glob '' -Path 'bin'
ForEach ($g in '*.vpk','*.cache') { Glob-Symlink -Glob $g -Path 'tf' }
