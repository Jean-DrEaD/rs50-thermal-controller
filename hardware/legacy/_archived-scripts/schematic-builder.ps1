# schematic-builder.ps1
# Funcoes pra gerar S-expressions do KiCad 8/9 schematic

function New-Uuid {
    return [guid]::NewGuid().ToString().ToLower()
}

function New-SchematicHeader {
    $uuid = New-Uuid
    return @"
(kicad_sch
	(version 20231120)
	(generator "rs50-custom")
	(generator_version "8.0")
	(uuid "$uuid")
	(paper "A4")
	(title_block
		(title "RS50 Thermal Controller")
		(date "2026-05-01")
		(rev "v0.1")
		(company "Delano - MyHUB.IA")
		(comment 1 "Controlador termico para resistencia 50W")
		(comment 2 "ESP32-S3-Zero + NTC + Rele + MOSFET")
	)
	(lib_symbols
"@
}

function New-LibSymbolsSection {
    # Esta secao deveria conter os simbolos cacheados.
    # KiCad popula isso automaticamente ao abrir/salvar.
    # Vamos deixar vazio - KiCad vai preencher na primeira abertura.
    return @"
	)
"@
}

function New-Component {
    param($comp)

    $uuid = New-Uuid
    $rot = if ($comp.Rot) { $comp.Rot } else { 0 }

    $sb = [System.Text.StringBuilder]::new()
    [void]$sb.AppendLine("	(symbol")
    [void]$sb.AppendLine("		(lib_id `"$($comp.LibId)`")")
    [void]$sb.AppendLine("		(at $($comp.X) $($comp.Y) $rot)")
    [void]$sb.AppendLine("		(unit 1)")
    [void]$sb.AppendLine("		(exclude_from_sim no)")
    [void]$sb.AppendLine("		(in_bom yes)")
    [void]$sb.AppendLine("		(on_board yes)")
    [void]$sb.AppendLine("		(dnp no)")
    [void]$sb.AppendLine("		(uuid `"$uuid`")")
    [void]$sb.AppendLine("		(property `"Reference`" `"$($comp.Ref)`"")
    [void]$sb.AppendLine("			(at $($comp.X) $($comp.Y - 8) 0)")
    [void]$sb.AppendLine("			(effects (font (size 1.27 1.27)))")
    [void]$sb.AppendLine("		)")
    [void]$sb.AppendLine("		(property `"Value`" `"$($comp.Value)`"")
    [void]$sb.AppendLine("			(at $($comp.X) $($comp.Y + 8) 0)")
    [void]$sb.AppendLine("			(effects (font (size 1.27 1.27)))")
    [void]$sb.AppendLine("		)")
    [void]$sb.AppendLine("		(property `"Footprint`" `"$($comp.Footprint)`"")
    [void]$sb.AppendLine("			(at $($comp.X) $($comp.Y) 0)")
    [void]$sb.AppendLine("			(effects (font (size 1.27 1.27)) hide yes)")
    [void]$sb.AppendLine("		)")
    [void]$sb.AppendLine("		(property `"Datasheet`" `"`"")
    [void]$sb.AppendLine("			(at $($comp.X) $($comp.Y) 0)")
    [void]$sb.AppendLine("			(effects (font (size 1.27 1.27)) hide yes)")
    [void]$sb.AppendLine("		)")
    [void]$sb.AppendLine("		(property `"Description`" `"`"")
    [void]$sb.AppendLine("			(at $($comp.X) $($comp.Y) 0)")
    [void]$sb.AppendLine("			(effects (font (size 1.27 1.27)) hide yes)")
    [void]$sb.AppendLine("		)")
    [void]$sb.AppendLine("		(instances")
    [void]$sb.AppendLine("			(project `"rs50-thermal`"")
    [void]$sb.AppendLine("				(path `"/$(New-Uuid)`"")
    [void]$sb.AppendLine("					(reference `"$($comp.Ref)`")")
    [void]$sb.AppendLine("					(unit 1)")
    [void]$sb.AppendLine("				)")
    [void]$sb.AppendLine("			)")
    [void]$sb.AppendLine("		)")
    [void]$sb.AppendLine("	)")
    return $sb.ToString()
}

function New-LocalLabel {
    param([string]$text, [double]$x, [double]$y, [int]$rot = 0)
    $uuid = New-Uuid
    return @"
	(label "$text"
		(at $x $y $rot)
		(effects (font (size 1.27 1.27)) (justify left bottom))
		(uuid "$uuid")
	)
"@
}

function New-PowerLabel {
    param([string]$net, [double]$x, [double]$y)
    # Power labels usam simbolo especial power:+24V, power:GND etc
    $uuid = New-Uuid
    $libId = "power:$net"
    $sb = [System.Text.StringBuilder]::new()
    [void]$sb.AppendLine("	(symbol")
    [void]$sb.AppendLine("		(lib_id `"$libId`")")
    [void]$sb.AppendLine("		(at $x $y 0)")
    [void]$sb.AppendLine("		(unit 1)")
    [void]$sb.AppendLine("		(exclude_from_sim no)")
    [void]$sb.AppendLine("		(in_bom yes)")
    [void]$sb.AppendLine("		(on_board yes)")
    [void]$sb.AppendLine("		(dnp no)")
    [void]$sb.AppendLine("		(uuid `"$uuid`")")
    [void]$sb.AppendLine("		(property `"Reference`" `"#PWR`"")
    [void]$sb.AppendLine("			(at $x $($y - 5) 0)")
    [void]$sb.AppendLine("			(effects (font (size 1.27 1.27)) hide yes)")
    [void]$sb.AppendLine("		)")
    [void]$sb.AppendLine("		(property `"Value`" `"$net`"")
    [void]$sb.AppendLine("			(at $x $($y - 3) 0)")
    [void]$sb.AppendLine("			(effects (font (size 1.27 1.27)))")
    [void]$sb.AppendLine("		)")
    [void]$sb.AppendLine("		(instances")
    [void]$sb.AppendLine("			(project `"rs50-thermal`"")
    [void]$sb.AppendLine("				(path `"/$(New-Uuid)`"")
    [void]$sb.AppendLine("					(reference `"#PWR?`")")
    [void]$sb.AppendLine("					(unit 1)")
    [void]$sb.AppendLine("				)")
    [void]$sb.AppendLine("			)")
    [void]$sb.AppendLine("		)")
    [void]$sb.AppendLine("	)")
    return $sb.ToString()
}

function New-SchematicFooter {
    return @"
	(sheet_instances
		(path "/"
			(page "1")
		)
	)
)
"@
}

Write-Host "schematic-builder.ps1 carregado: funcoes prontas" -ForegroundColor Green
