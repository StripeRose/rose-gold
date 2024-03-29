using System.IO;
using Sharpmake;

[module: Sharpmake.Include("../.sharpmake/coreproject.sharpmake.cs")]

[Generate]
public class DirectXTex : RoseGold.Core.ExternalLibrary
{
    public DirectXTex()
    {
        Name = "DirectXTex";
        SourceRootPath = Path.Combine("[project.SharpmakeCsPath]", "DirectXTex", "DirectXTex");
    }

    public override void ConfigureAll(Configuration conf, Target target)
    {
        base.ConfigureAll(conf, target);

        conf.SolutionFolder = "rose-gold/external";
        conf.ProjectPath = "[project.SharpmakeCsPath]";
        conf.SourceFilesBuildExclude.Add("BCDirectCompute.cpp");
    }
}
