"""Renderiza os guias Markdown atuais em um PDF dark com navegação interna.

Uso: python scripts/gerar_guia.py [--output caminho.pdf]
"""
from pathlib import Path
import argparse
import html
import re
import textwrap
import os
import json
import hashlib
from datetime import datetime
from zoneinfo import ZoneInfo

from reportlab.pdfgen import canvas
from reportlab.platypus import (BaseDocTemplate, PageTemplate, Frame, Paragraph,
    Spacer, PageBreak, Table, TableStyle, Preformatted, Image, KeepTogether)
from reportlab.platypus.tableofcontents import TableOfContents
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.colors import HexColor
from reportlab.lib.enums import TA_LEFT
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.lib.pagesizes import A4

ROOT = Path(__file__).resolve().parents[1]
BG, PANEL, INK, MUTED, GREEN = map(HexColor, ['#101714','#18241e','#e2eee5','#a4bbae','#67ef73'])
W,H = A4
WIDTH = W-88

def fonts():
    fontdir=Path('C:/Windows/Fonts')
    candidates=[(fontdir,'segoeui.ttf','segoeuib.ttf','consola.ttf'),
        (Path('/usr/share/fonts/truetype/dejavu'),'DejaVuSans.ttf','DejaVuSans-Bold.ttf','DejaVuSansMono.ttf')]
    for directory,regular,bold,mono in candidates:
        if (directory/regular).exists():
            for name,file in [('Body',regular),('Strong',bold),('Code',mono)]:
                pdfmetrics.registerFont(TTFont(name,str(directory/file)))
            pdfmetrics.registerFontFamily('Body',normal='Body',bold='Strong',italic='Body',boldItalic='Strong')
            return
    raise RuntimeError('Instale as fontes Segoe UI/Consolas ou DejaVu Sans/Mono.')

def inline(s):
    s=html.escape(s)
    s=re.sub(r'`([^`]+)`',r'<font name="Code" color="#94efa1">\1</font>',s)
    s=re.sub(r'\*\*([^*]+)\*\*',r'<b>\1</b>',s)
    s=re.sub(r'\[([^\]]+)\]\(([^)]+)\)',lambda m: m[1] if m[2].startswith('#') else f'<link href="{m[2]}" color="#67ef73">{m[1]}</link>',s)
    return s

class Guide(BaseDocTemplate):
    def afterFlowable(self,flow):
        if hasattr(flow,'bookmark'):
            level,title,key=flow.bookmark
            self.canv.bookmarkPage(key)
            self.canv.addOutlineEntry(title,key,level=level,closed=False)
            self.notify('TOCEntry',(level,title,self.page,key))

def page(c,doc):
    c.setFillColor(BG); c.rect(0,0,W,H,fill=1,stroke=0)
    if doc.page==1: return
    c.setStrokeColor(HexColor('#2c4435')); c.line(44,H-41,W-44,H-41)
    c.setFont('Strong',8); c.setFillColor(GREEN); c.drawString(44,H-31,'DISPLAYWX  /  GUIA DA BIBLIOTECA')
    c.setFont('Body',8); c.setFillColor(MUTED)
    c.drawString(44,26,'Guia técnico • ESP32 • Widgets • Câmera e DMA')
    c.drawRightString(W-44,26,str(doc.page))
    c.linkAbsolute('Voltar ao índice','indice',(44,19,260,38),thickness=0)

def generate(output):
    fonts()
    styles={
        'body':ParagraphStyle('body',fontName='Body',fontSize=10,leading=15,textColor=INK,spaceAfter=9),
        'h1':ParagraphStyle('h1',fontName='Strong',fontSize=25,leading=31,textColor=GREEN,spaceAfter=18),
        'h2':ParagraphStyle('h2',fontName='Strong',fontSize=18,leading=24,textColor=GREEN,spaceBefore=14,spaceAfter=12,keepWithNext=True),
        'h3':ParagraphStyle('h3',fontName='Strong',fontSize=12,leading=18,textColor=INK,spaceBefore=12,spaceAfter=8,keepWithNext=True),
        'cell':ParagraphStyle('cell',fontName='Body',fontSize=8,leading=11,textColor=INK,wordWrap='CJK'),
        'code':ParagraphStyle('code',fontName='Code',fontSize=7.1,leading=10,textColor=HexColor('#b6e9c3'),backColor=PANEL,borderPadding=9,spaceBefore=7,spaceAfter=12),
    }
    story=[]
    story += [Spacer(1,35),Image(str(ROOT/'docs/assets/displaywx-icon.png'),110,110,hAlign='LEFT'),Spacer(1,25)]
    story += [Paragraph('DISPLAYWX',ParagraphStyle('brand',fontName='Strong',fontSize=43,leading=52,textColor=GREEN)),
        Paragraph('Guia da biblioteca',ParagraphStyle('subtitle',fontName='Body',fontSize=25,leading=35,textColor=INK)),Spacer(1,22),
        Paragraph('Interfaces gráficas para ESP32',styles['h3']),
        Paragraph('Telas, toque, widgets e câmera MIPI.<br/>Com exemplos comentados e guia de DMA.',styles['body']),Spacer(1,55)]
    props=(ROOT/'library.properties').read_text(encoding='utf-8')
    version=re.search(r'^version=(.*)',props,re.M)
    try: date=datetime.now(ZoneInfo('America/Sao_Paulo')).strftime('%d/%m/%Y')
    except Exception: date=datetime.now().strftime('%d/%m/%Y')
    story += [Paragraph(f'Biblioteca {version[1] if version else "DisplayWX"}  •  Edição {date}',styles['body']),
        Paragraph('Manual navegável<br/>Use o sumário e os marcadores do leitor de PDF.<br/>Clique no rodapé para voltar ao índice.',styles['body']),PageBreak()]
    toc=TableOfContents()
    toc.levelStyles=[ParagraphStyle('toc0',fontName='Strong',fontSize=11,leading=17,textColor=GREEN,spaceBefore=10),ParagraphStyle('toc1',fontName='Body',fontSize=9,leading=14,textColor=INK,leftIndent=12,spaceBefore=3)]
    story += [Paragraph('<a name="indice"/>Sumário clicável',styles['h1']),Paragraph('Selecione um capítulo para ir diretamente ao conteúdo.',styles['body']),toc,PageBreak()]
    headings=[]
    sources=sorted((ROOT/'docs').glob('GUIA_*.md'),key=lambda p:(p.name!='GUIA_DISPLAYWX.md',p.name))
    if not sources:raise RuntimeError('Nenhum docs/GUIA_*.md encontrado.')
    for section,source in enumerate(sources):
        lines=source.read_text(encoding='utf-8-sig').splitlines(); i=0; skip_toc=False
        while i<len(lines):
            line=lines[i]; i+=1
            if line.strip()=='[TOC]':continue
            if line.startswith('## Sumário'):skip_toc=True;continue
            if skip_toc:
                if line.startswith('## ') and not line.startswith('## Sumário'):skip_toc=False
                else:continue
            if not line.strip() or re.fullmatch(r'\s*[-*_]{3,}\s*',line):continue
            if line.startswith('```'):
                code=[]
                while i<len(lines) and not lines[i].startswith('```'):
                    raw=lines[i].expandtabs(4)
                    code += textwrap.wrap(raw,width=104,replace_whitespace=False,drop_whitespace=False,break_long_words=True,break_on_hyphens=False,subsequent_indent='    ') or ['']
                    i+=1
                i+=1
                story.append(Preformatted('\n'.join(code),styles['code']))
                continue
            match=re.match(r'^(#{1,6})\s+(.+)',line)
            if match:
                depth=len(match[1]); title=match[2].replace('`','')
                if depth<=2:
                    if depth==1 and section:story.append(PageBreak())
                    elif depth==2:story.append(Spacer(1,6))
                    key=f'section-{section}-{len(headings)}';level=0 if depth==1 else 1
                    p=Paragraph(inline(title),styles['h1' if depth==1 else 'h2']);p.bookmark=(level,title,key)
                    headings.append((title,key));story.append(p)
                else:story.append(Paragraph(inline(title),styles['h3']))
                continue
            if line.startswith('|'):
                rows=[line]
                while i<len(lines) and lines[i].startswith('|'):rows.append(lines[i]);i+=1
                cells=[]
                for row in rows:
                    vals=row.strip().strip('|').split('|')
                    if all(re.fullmatch(r'\s*:?-+:?\s*',x) for x in vals):continue
                    cells.append([Paragraph(inline(x.strip()),styles['cell']) for x in vals])
                cols=max(map(len,cells)); cells=[r+[Paragraph('',styles['cell'])]*(cols-len(r)) for r in cells]
                t=Table(cells,colWidths=[WIDTH/cols]*cols,repeatRows=1,hAlign='LEFT')
                t.setStyle(TableStyle([('BACKGROUND',(0,0),(-1,0),HexColor('#245c36')),('ROWBACKGROUNDS',(0,1),(-1,-1),[PANEL,HexColor('#142019')]),('VALIGN',(0,0),(-1,-1),'TOP'),('LEFTPADDING',(0,0),(-1,-1),7),('RIGHTPADDING',(0,0),(-1,-1),7),('TOPPADDING',(0,0),(-1,-1),7),('BOTTOMPADDING',(0,0),(-1,-1),7),('LINEBELOW',(0,0),(-1,0),0.7,GREEN)]))
                story.extend([t,Spacer(1,12)]);continue
            bullet=re.match(r'^\s*(?:[-*]|\d+\.)\s+(.+)',line)
            if bullet:story.append(Paragraph('• '+inline(bullet[1]),styles['body']));continue
            para=[line]
            while i<len(lines) and lines[i].strip() and not re.match(r'^(#|```|\||[-*]\s|\d+\.\s|---)',lines[i]):
                para.append(lines[i]);i+=1
            story.append(Paragraph(inline(' '.join(para)),styles['body']))
    output.parent.mkdir(parents=True,exist_ok=True)
    temporary=output.with_suffix('.building.pdf')
    doc=Guide(str(temporary),pagesize=A4,leftMargin=44,rightMargin=44,topMargin=61,bottomMargin=52,title='DisplayWX - Guia da biblioteca',author='DisplayWX',pageCompression=1)
    frame=Frame(44,52,WIDTH,H-113,leftPadding=0,rightPadding=0,topPadding=0,bottomPadding=0)
    doc.addPageTemplates(PageTemplate(id='dark',frames=frame,onPage=page))
    doc.multiBuild(story)
    from pypdf import PdfReader
    r=PdfReader(temporary)
    annotations=sum(len(p.get('/Annots',[])) for p in r.pages)
    if annotations<len(headings):raise RuntimeError('Falha na navegação interna.')
    os.replace(temporary,output)
    manifest={'sources':{str(p.relative_to(ROOT)).replace('\\','/'):hashlib.sha256(p.read_bytes()).hexdigest() for p in sources},'pdf_sha256':hashlib.sha256(output.read_bytes()).hexdigest(),'pages':len(r.pages),'links':annotations}
    output.with_suffix('.manifest.json').write_text(json.dumps(manifest,indent=2,ensure_ascii=False),encoding='utf-8')
    print(f'PDF: {output}\nPáginas: {len(r.pages)} | Capítulos: {len(headings)} | Links: {annotations}')

def validate(output):
    from pypdf import PdfReader
    manifest=json.loads(output.with_suffix('.manifest.json').read_text(encoding='utf-8'))
    sources=sorted((ROOT/'docs').glob('GUIA_*.md'))
    current={str(p.relative_to(ROOT)).replace('\\','/'):hashlib.sha256(p.read_bytes()).hexdigest() for p in sources}
    if manifest['sources']!=current:raise RuntimeError('PDF não corresponde aos Markdown atuais.')
    if manifest['pdf_sha256']!=hashlib.sha256(output.read_bytes()).hexdigest():raise RuntimeError('PDF alterado após geração.')
    reader=PdfReader(output)
    assert len(reader.pages)==manifest['pages'] and reader.outline
    print(f'Validação OK: {len(reader.pages)} páginas; fontes atuais e navegação presentes.')

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output',type=Path,default=ROOT/'output/pdf/Guia_DisplayWX.pdf')
    parser.add_argument('--validate',action='store_true',help='Verifica PDF e hashes dos Markdown sem regenerar.')
    args=parser.parse_args()
    (validate if args.validate else generate)(args.output.resolve())
