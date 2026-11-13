import { makeNativeBulletParagraphs } from 'file:///C:/Users/Anglo/.codex/plugins/cache/openai-primary-runtime/presentations/26.1112.11011/skills/presentations/container_tools/artifact_tool_utils.mjs';

const C = { bg:'#1B1D1E', green:'#00DE00', jade:'#00AD5A', gray:'#5E6D70', panel:'#424E4F', text:'#F3F6F4', muted:'#BAC6C5', code:'#111516', cyan:'#74D9E8', amber:'#F2C75B' };
function box(s,x,y,w,h,fill,stroke='none',geometry='rect') {
  return s.shapes.add({geometry,position:{left:x,top:y,width:w,height:h},fill,line:{fill:stroke,width:stroke==='none'?0:2}});
}
function txt(s,text,x,y,w,h,size=24,color=C.text,bold=false,font='Aptos',align='left') {
  const a=box(s,x,y,w,h,'none');a.text=text;
  a.text.style={fontSize:size,typeface:font,color,bold,alignment:align,verticalAlignment:'top',autoFit:'none',wrap:'square',insets:0};return a;
}
function path(s,points,x,y,w,h,color,width=3) {
  return s.shapes.add({geometry:'custom',position:{left:x,top:y,width:w,height:h},fill:'none',line:{fill:color,width,style:'solid'},customPaths:[{width:w,height:h,commands:points.map(([px,py],i)=>i?{lineTo:{x:px,y:py}}:{moveTo:{x:px,y:py}})}]});
}
function comparison(s,x,enabled) {
  txt(s,enabled?'CONFIGURAÇÃO ATUAL':'COMPARAÇÃO COM FALSE',x,153,556,27,19,enabled?C.green:C.muted,true);
  box(s,x,186,556,54,C.code,C.gray);
  const snippet='.workInBackground = '+(enabled?'true':'false');
  const c=txt(s,snippet,x+18,197,520,35,27,C.text,false,'Consolas');
  c.text.getRange(1,16).fill=C.green;
  c.text.getRange(snippet.indexOf(enabled?'true':'false'),enabled?4:5).fill=C.cyan;
  txt(s,'screen0',x+2,257,106,28,21,C.text,false,'Aptos','center');
  txt(s,'Prog1 / Prog2',x+133,257,286,28,21,C.muted,false,'Aptos','center');
  txt(s,'screen0',x+449,257,106,28,21,C.text,false,'Aptos','center');
  box(s,x+128,289,290,142,'#293132');
  const pts=[[54,118],[166,76],[278,34],[390,87],[502,107]];
  if(enabled)path(s,pts,x,288,556,145,C.green,4);
  const vals=['800','1200','1600','1100','900'];
  pts.forEach(([px,py],i)=>{
    const accepted=enabled||i===0||i===4;
    txt(s,vals[i],x+px-37,288+py-36,74,28,22,accepted?C.text:C.muted,false,'Aptos','center');
    if(accepted)box(s,x+px-7,288+py-7,14,14,C.green,'none','ellipse');
    else {
      path(s,[[0,0],[16,16]],x+px-8,288+py-8,16,16,C.amber,3);
      path(s,[[0,16],[16,0]],x+px-8,288+py-8,16,16,C.amber,3);
    }
  });
  txt(s,enabled?'Retorno com amostras da navegação':'Retorno sem amostras da navegação',x,447,556,36,25,enabled?C.green:C.amber,true);
  txt(s,enabled?'Pontos verdes: RPM armazenado.':'Cruzes: RPM ignorado enquanto a tela estava oculta.',x,485,556,34,20,C.muted);
}

export function addWorkInBackgroundSlide(presentation,number=17) {
  const s=presentation.slides.add({width:1280,height:720});s.background.fill=C.bg;
  box(s,56,48,5,64,C.green);
  txt(s,'workInBackground em configLineChart0',82,43,1134,83,40,C.text,true);
  txt(s,'Amostras aceitas por graphfreq ao sair da tela principal e voltar',58,120,1164,33,24,C.muted);
  comparison(s,58,true);comparison(s,666,false);
  box(s,58,528,1164,3,C.jade);
  const b=txt(s,'',65,543,1150,112,21);
  b.text=makeNativeBulletParagraphs([
    'Histórico limitado à capacidade do gráfico. O desenho ocorre na tela visível.',
    'Não controla programas. Sem telemetria nova e válida, não há novos pontos.',
    'Prog1 e Prog2 usam false: cada gráfico aceita pontos apenas na própria tela.'
  ],{marginLeftPoints:16,hangingPoints:10,spaceAfterPoints:8});
  txt(s,'GRÁFICOS  ·  Exemplo ilustrativo de RPM, sem escala temporal',58,665,1080,24,15,C.muted);
  txt(s,String(number).padStart(2,'0'),1170,662,56,28,18,C.green,true,'Aptos','right');
  s.speakerNotes.text='Fontes: C:/arduino_edgewx/portable/sketchbook/libraries/displaywx/examples/advanced/motor_trifasico/codigo_display_7in/README.md, seções 4.1, 5 e 13; codigo_display_7in.ino, configurações configLineChart0/1/2; fluxo.h, servicoInterface(); src/widgets/linechart/wlinechart.cpp, LineChart::push(). Revisão em 13/11/2026.\nconfigLineChart0, aplicado a graphfreq de screen0, usa workInBackground=true. As amostras novas e válidas oferecidas pelo serviço gráfico continuam entrando na série 0 durante a navegação por Prog1 e Prog2. Ao voltar à tela principal, o desenho mostra o histórico dentro da capacidade do buffer. Com false, push() retorna sem inserir quando a tela do widget está oculta; amostras recusadas não são recuperadas posteriormente. As configurações configLineChart1 e configLineChart2 usam false. O parâmetro não inicia nem interrompe programas e não produz amostras sem telemetria.\nAs figuras comparam a mesma sequência ilustrativa de RPM (800, 1200, 1600, 1100, 900). A screen0 está visível na primeira e na última amostra e oculta nas três intermediárias. O exemplo assume espaço suficiente no buffer e que essas amostras são oferecidas ao gráfico. As cruzes representam rejeição na entrada, não lacunas temporais desenhadas pelo widget real. O eixo horizontal é apenas ordem conceitual de chegada. A coluna false é uma comparação hipotética para configLineChart0, não a configuração atual do projeto.\nVerificação proposta no README: receber telemetria em Prog1/Prog2 e voltar à tela principal, confirmando que graphfreq inclui as amostras recebidas durante a navegação, dentro de sua capacidade.';
  return s;
}

export async function updatePresentation({presentation}) {
  const original=(await presentation.inspect({select:'$.slides[*]',include:['id']})).records.map(r=>r.id);
  if(original.length!==36)throw new Error('Esperados 36 slides antes da inserção.');
  presentation.slides.getItem(10).speakerNotes.text+='\nREADME atualizado: graphfreq armazena RPM na série 0 com workInBackground=true, inclusive quando screen0 está oculta, dentro da capacidade do gráfico.';
  presentation.slides.getItem(14).speakerNotes.text+='\nO serviço oferece RPM novo e válido à série 0 dos três gráficos. Cada gráfico aceita pontos conforme workInBackground: configLineChart0=true e configLineChart1/2=false. O desenho só ocorre na tela visível.';
  presentation.slides.getItem(23).speakerNotes.text+='\nA atualização de medidores oferece RPM aos três gráficos. A aceitação depende de workInBackground e da tela ativa: o principal usa true e os programas usam false.';
  presentation.slides.getItem(24).speakerNotes.text+='\nPara rpm, o serviço oferece a amostra à série 0 dos três gráficos. graphfreq aceita também fora da tela principal; gráficos dos programas aceitam apenas enquanto a própria tela está ativa.';
  presentation.slides.getItem(32).speakerNotes.text+='\nNovo cenário da seção 13: receber telemetria em Prog1 ou Prog2 e retornar à tela principal. Espera-se graphfreq com as amostras recebidas durante a navegação, dentro da capacidade, pois configLineChart0.workInBackground=true. Este cenário é uma verificação proposta.';
  const diag=presentation.resolve('sh/8z2hozu9');
  diag.text=makeNativeBulletParagraphs([
    'Atualizar UI e trocar telas: imagem alinhada, sem deslocamento vertical',
    'Receber RPM em Prog1/Prog2 e voltar: histórico principal inclui as amostras',
    'Boot sem Wi-Fi: navegação e touch continuam',
    'Queda e reconexão: comandos antigos não reaparecem',
    'cmd=50 do ID 3: slider muda sem eco',
    'Sem telemetria: frequências mostram -- após 5 s'
  ],{marginLeftPoints:16,hangingPoints:10,spaceAfterPoints:12});
  diag.position={left:74,top:330,width:1120,height:306};
  diag.text.style={fontSize:25,typeface:'Aptos',color:C.text,alignment:'left',verticalAlignment:'top',autoFit:'none',wrap:'square',insets:0};
  addWorkInBackgroundSlide(presentation);
  const all=(await presentation.inspect({select:'$.slides[*]',include:['id']})).records.map(r=>r.id);
  const newId=all[all.length-1];
  const order=[...original.slice(0,16),newId,...original.slice(16)];
  presentation.slides.reorder(order);
  const text=(await presentation.inspect({kind:'textbox',maxChars:200000})).records;
  for(const r of text)if(r.position?.left===1170&&r.position?.top===662)presentation.resolve(r.id).text=String(r.slideIndex+1).padStart(2,'0');
  await presentation.render({slide:[16,33]});
  return {slides:presentation.slides.items.length,newSlide:17,newId};
}
