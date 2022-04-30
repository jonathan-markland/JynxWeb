open System
open System.IO



let WithExceptionReportingToStdOutDo resultIfError action =
    try
        action ()
    with e ->
        Console.Error.WriteLine $"Error: {e.Message}"
        resultIfError



let CreateFileUsing contentGenerator filePath =    
    
    let fileWriter = 
        File.CreateText filePath

    let writeToDestinationFile s = 
        fileWriter.Write(s:string)

    try
        contentGenerator writeToDestinationFile
        fileWriter.Close()
    with _ ->
        fileWriter.Close()
        File.Delete filePath
        reraise ()



let LynxBasicLoadCommandFor (tapFileName:string) =

    let trim startLength =
        let trimmedName = tapFileName.Substring(startLength)
        if trimmedName.ToLower().EndsWith(".tap") then
            trimmedName.Substring(0, trimmedName.Length-4)
        else
            trimmedName

    if tapFileName.StartsWith("LOAD ") then
        $"LOAD \"{trim 5}\""

    else if tapFileName.StartsWith("MLOAD ") then
        $"MLOAD \"{trim 6}\""

    else
        "REM Cannot load: " + tapFileName



let EscapeForC str =
    (str:string).Replace("\"", "\\\"")



let AsCppBool boolValue =
    if boolValue = true then "true" else "false"



let GameNeedsLevel9Palette (tapFileName:string) =
    ["LOAD COLOSSAL.TAP" ; "LOAD DUNGEON.TAP"] |> List.contains tapFileName
        



let WriteFileBinaryUsing writeAction name (fileBinary:byte[]) =

    let fileLength =
        fileBinary.Length

    let fileLengthStr =
        sprintf "%d" fileLength

    let writeLine s = 
        writeAction s
        writeAction (System.Environment.NewLine)

    let lynxBasicLoadCommand =
        LynxBasicLoadCommandFor name

    writeLine "//"    
    writeLine $"// {lynxBasicLoadCommand}"
    writeLine "//"    
    writeLine ""    

    writeLine $"#define BUILT_IN_CASSETTE_IMAGE_LENGTH {fileLengthStr}"
    writeLine $"const char *BuiltInCassetteLoadCommnad = \"{lynxBasicLoadCommand |> EscapeForC}\";"
    writeLine $"unsigned int BuiltInCassetteImageLength = BUILT_IN_CASSETTE_IMAGE_LENGTH;"
    writeLine $"bool BuiltInCassetteNeedsLevel9Palette = {GameNeedsLevel9Palette name |> AsCppBool};"
    writeLine $"unsigned char BuiltInCassetteImage[BUILT_IN_CASSETTE_IMAGE_LENGTH] = {{"
    writeLine ""    
    
    for i in 0..fileLength-1 do
        let column = (i % 16)
        if column = 0 then writeAction "    "
        writeAction (sprintf "0x%02x, " fileBinary[i])
        if column = 15 then writeLine ""

    writeLine ""    
    writeLine $"}};"
    writeLine ""    

    

let ProcessSourceFileToTargetFile sourceFile destFile =

    let fileBytes = File.ReadAllBytes sourceFile

    let name = Path.GetFileName(sourceFile)

    let contentGenerator writeAction =
        WriteFileBinaryUsing writeAction name fileBytes

    CreateFileUsing contentGenerator destFile






[<EntryPoint>]
let main argv =

    WithExceptionReportingToStdOutDo 1 (fun () ->

        if argv.Length = 2 then
            let sourceFile = argv[0]
            let destFile = argv[1]
            ProcessSourceFileToTargetFile sourceFile destFile
            0

        else if argv.Length = 1 then
            let sourceFolder = argv[0]
            let files = System.IO.Directory.EnumerateFiles(sourceFolder, "*.tap")
            files |> Seq.iter (fun sourceFile -> 
                let leafName = Path.GetFileName(sourceFile)
                let destFile = Path.Combine(sourceFolder, "CppTapeFiles", leafName + ".cpp")
                ProcessSourceFileToTargetFile sourceFile destFile
            )
            0

        else
            Console.Error.WriteLine "Error: Usage: <source file> <destination file>"
            1
    )
