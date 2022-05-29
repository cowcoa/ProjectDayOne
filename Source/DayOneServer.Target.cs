// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class DayOneServerTarget : TargetRules
{
	public DayOneServerTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;

		//bUseUnityBuild = false;
		//bUsePCHFiles = false;

		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "DayOne" } );
	}
}
