Add-Type -AssemblyName System.Windows.Forms.DataVisualization
Add-Type -AssemblyName System.Drawing
$root=Split-Path $PSScriptRoot -Parent
$out=Join-Path $root 'docs\assets'
New-Item -ItemType Directory -Path $out -Force | Out-Null
$rows=@(Get-ChildItem (Join-Path $root 'data\final-series\*.json') | ForEach-Object {Get-Content $_.FullName -Raw|ConvertFrom-Json})
if($rows.Count -ne 30){throw 'Expected 30 captures'}
$colors=@{apple='#B84A48';potato='#9C741C';cucumber='#28786B'}
$names=@{apple='Apple';potato='Potato';cucumber='Cucumber'}
function New-Plot($title,$xt,$yt){
$c=New-Object System.Windows.Forms.DataVisualization.Charting.Chart
$c.Width=1400;$c.Height=850;$c.BackColor=[Drawing.ColorTranslator]::FromHtml('#FFFFFF')
$a=New-Object System.Windows.Forms.DataVisualization.Charting.ChartArea 'main'
$a.BackColor=[Drawing.ColorTranslator]::FromHtml('#FFFFFF');$a.AxisX.Title=$xt;$a.AxisY.Title=$yt
foreach($axis in @($a.AxisX,$a.AxisY)){$axis.TitleFont=New-Object Drawing.Font('Arial',16);$axis.LabelStyle.Font=New-Object Drawing.Font('Arial',12);$axis.MajorGrid.LineColor=[Drawing.Color]::FromArgb(220,225,230);$axis.LabelStyle.ForeColor=[Drawing.Color]::FromArgb(30,40,50);$axis.TitleForeColor=[Drawing.Color]::FromArgb(30,40,50);$axis.LineColor=[Drawing.Color]::SlateGray;$axis.MajorTickMark.LineColor=[Drawing.Color]::SlateGray}
$c.ChartAreas.Add($a)
$t=$c.Titles.Add($title);$t.Font=New-Object Drawing.Font('Arial',20);$t.ForeColor=[Drawing.Color]::FromArgb(30,40,50)
$l=New-Object System.Windows.Forms.DataVisualization.Charting.Legend 'legend';$l.BackColor=[Drawing.Color]::Transparent;$l.ForeColor=[Drawing.Color]::FromArgb(30,40,50);$l.Docking='Bottom';$l.Font=New-Object Drawing.Font('Arial',14);$c.Legends.Add($l)
return ,$c
}
$c=New-Plot '30 placements: two per specimen, 15 specimens' 'Forward pulse at 1 ms (mV)' 'Reverse pulse at 1 ms (mV)'
foreach($cl in @('potato','cucumber','apple')){
$s=New-Object System.Windows.Forms.DataVisualization.Charting.Series $names[$cl];$s.ChartType='Point';$s.MarkerSize=11;$s.Color=[Drawing.ColorTranslator]::FromHtml($colors[$cl]);$s.MarkerStyle='Circle'
foreach($r in @($rows|Where-Object {$_.class -eq $cl})){[void]$s.Points.AddXY([double]$r.features_mv[0],[double]$r.features_mv[3])};$c.Series.Add($s)
}
$c.ChartAreas[0].AxisX.Minimum=300;$c.ChartAreas[0].AxisX.Maximum=1700;$c.ChartAreas[0].AxisX.Interval=200;$c.ChartAreas[0].AxisX.LabelStyle.Format='0';$c.SaveImage((Join-Path $out 'scatter.png'),'Png');$c.Dispose()
$c=New-Plot 'Forward response: one line per electrode placement' 'Time after pulse start (ms)' 'GPIO4 voltage (mV)'
$seen=@{}
foreach($r in $rows){$s=New-Object System.Windows.Forms.DataVisualization.Charting.Series ($r.specimen_id+'-'+$r.installation);$s.ChartType='Line';$s.BorderWidth=2;$s.MarkerStyle='Circle';$s.MarkerSize=6;$s.Color=[Drawing.Color]::FromArgb(140,[Drawing.ColorTranslator]::FromHtml($colors[$r.class]));$s.IsVisibleInLegend=!$seen.ContainsKey($r.class);$s.LegendText=$names[$r.class];$seen[$r.class]=$true;for($j=0;$j -lt 3;$j++){[void]$s.Points.AddXY(@(1,5,20)[$j],[double]$r.features_mv[$j])};$c.Series.Add($s)}
$c.ChartAreas[0].AxisX.Minimum=0;$c.ChartAreas[0].AxisX.Maximum=21
$c.SaveImage((Join-Path $out 'response.png'),'Png');$c.Dispose()
$c=New-Plot 'Electrode relocation: paired placements of each specimen' 'Specimen ID FINAL01–FINAL15' 'Forward pulse at 1 ms (mV)'
$seen=@{}
foreach($g in ($rows|Group-Object specimen_id|Sort-Object Name)){
$rr=@($g.Group|Sort-Object installation);$cl=$rr[0].class;$n=[int]$g.Name.Substring(5)
$s=New-Object System.Windows.Forms.DataVisualization.Charting.Series $g.Name;$s.ChartType='Line';$s.BorderWidth=3;$s.MarkerStyle='Circle';$s.MarkerSize=10;$s.Color=[Drawing.ColorTranslator]::FromHtml($colors[$cl]);$s.IsVisibleInLegend=!$seen.ContainsKey($cl);$s.LegendText=$names[$cl];$seen[$cl]=$true
[void]$s.Points.AddXY(($n-0.12),[double]$rr[0].features_mv[0]);[void]$s.Points.AddXY(($n+0.12),[double]$rr[1].features_mv[0]);$c.Series.Add($s)
}
$c.ChartAreas[0].AxisX.Minimum=0;$c.ChartAreas[0].AxisX.Maximum=16;$c.ChartAreas[0].AxisX.Interval=1
$c.SaveImage((Join-Path $out 'relocation.png'),'Png');$c.Dispose()


'3 charts and 2 photographs prepared.'

$c=New-Plot 'Training size: unchanged performance in this dataset' 'Training specimens (equal number per class)' 'Correct predictions on the final series (%)'
$a=$c.ChartAreas[0];$a.AxisX.Minimum=2;$a.AxisX.Maximum=7;$a.AxisX.Interval=1;$a.AxisY.Minimum=0;$a.AxisY.Maximum=110;$a.AxisY.Interval=25
$s=New-Object System.Windows.Forms.DataVisualization.Charting.Series 'Mean result';$s.ChartType='Line';$s.Color=[Drawing.ColorTranslator]::FromHtml('#28786B');$s.BorderWidth=5;$s.MarkerStyle='Circle';$s.MarkerSize=16
[void]$s.Points.AddXY(3,100);[void]$s.Points.AddXY(6,100)
$s.Points[0].Label='30/30 in all 8 combinations';$s.Points[1].Label='30/30: full training set';$s.Font=New-Object Drawing.Font('Arial',13);$s.LabelForeColor=[Drawing.Color]::FromArgb(30,40,50)
$c.Series.Add($s)
$c.SaveImage((Join-Path $out 'learning.png'),'Png');$c.Dispose()

