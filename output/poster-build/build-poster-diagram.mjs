import fs from 'node:fs/promises';
import path from 'node:path';
import { pathToFileURL } from 'node:url';
import { Presentation, PresentationFile } from 'file:///C:/Program%20Files/WindowsApps/OpenAI.CodexPrimaryRuntime.v26-1112-737-0_26.1112.737.0_x64__3k8sg7r9htsxt/dependencies/node/node_modules/@oai/artifact-tool/dist/artifact_tool.mjs';
const root='C:/Project/2026-CAPSTONE', build=`${root}/output/poster-build`;
const finalPath=`${root}/Reports/P_CAP_졸업작품전시_3장_도식형.pptx`;
const skill='C:/Users/MinSeok/.codex/plugins/cache/openai-primary-runtime/presentations/26.1112.11217/skills/presentations';
const S=.75, W=960, H=540, FONT='Arial';
const C={bg:'#F5F7FA',white:'#FFFFFF',navy:'#172B4D',blue:'#376B93',gold:'#C38A22',text:'#26364A',muted:'#65748B',line:'#CFD7E2',pale:'#E8EEF5',goldPale:'#F3E7C7',green:'#4A8A73'};
const p=Presentation.create({slideSize:{width:W,height:H}});
function rect(s,x,y,w,h,fill,stroke='none',sw=0){return s.shapes.add({geometry:'rect',position:{left:x*S,top:y*S,width:w*S,height:h*S},fill,line:{style:'solid',fill:stroke,width:sw}})}
function txt(s,v,x,y,w,h,size,color=C.text,bold=false,align='left'){
 const sh=s.shapes.add({geometry:'textbox',position:{left:x*S,top:y*S,width:w*S,height:h*S},fill:'none',line:{style:'solid',fill:'none',width:0}});
 sh.text=v;sh.text.style={fontFamily:FONT,fontSize:size*S,color,bold,verticalAlignment:'middle',textAlign:align};return sh;
}
function line(s,x1,y1,x2,y2,color=C.line,th=3){
 if(Math.abs(y2-y1)<1) rect(s,Math.min(x1,x2),y1-th/2,Math.abs(x2-x1),th,color);
 else if(Math.abs(x2-x1)<1) rect(s,x1-th/2,Math.min(y1,y2),th,Math.abs(y2-y1),color);
}
function header(s,topic,n){
 txt(s,`CAPSTONE PROJECT   /   ${topic.toUpperCase()}`,64,22,900,26,12,C.muted,true);
 rect(s,64,58,1152,2,C.gold);
 rect(s,64,670,1152,1,C.line);
 txt(s,'202104320 임민석  ·  졸업작품 전시회',64,680,850,22,11,C.muted,true);
 txt(s,`${String(n).padStart(2,'0')} / 03`,1090,680,126,22,11,C.muted,true,'right');
}
function mainTitle(s,t,sub){txt(s,t,64,82,1152,53,38,C.navy,true);txt(s,sub,64,139,1152,33,20,C.muted,false)}
function label(s,t,x,y,w){txt(s,t,x,y,w,28,18,C.blue,true);rect(s,x,y+34,w,1,C.line)}
function node(s,x,y,w,h,name,fill=C.white,stroke=C.line,color=C.text,size=15){rect(s,x,y,w,h,fill,stroke,2);txt(s,name,x+4,y+4,w-8,h-8,size,color,true,'center')}
function mapNode(s,x,y,name,type='room',size=52){
 const fill=type==='start'?C.gold:type==='boss'?C.navy:type==='special'?C.goldPale:C.white;
 const color=type==='start'||type==='boss'?C.white:C.text;
 node(s,x,y,size,size,name,fill,type==='room'?C.line:fill,color,13);
}
function connector(s,x1,y1,x2,y2,color=C.line){
 if(Math.abs(y1-y2)<1) rect(s,Math.min(x1,x2),y1-2,Math.abs(x2-x1),4,color);
 else if(Math.abs(x1-x2)<1) rect(s,x1-2,Math.min(y1,y2),4,Math.abs(y2-y1),color);
}
// Slide 1: procedural map generation, grounded in the first-semester report.
{
 const s=p.slides.add();s.background.fill=C.bg;header(s,'PROJECT OVERVIEW',1);
 mainTitle(s,'로그라이크 액션 게임','절차적 맵 생성  ·  배치 규칙으로 매번 다른 탐험 구조를 구성');
 label(s,'맵 생성 방식 개선',64,211,442);
 txt(s,'초기 랜덤 확장에서는 방이 네모난 형태로 몰리는 경향이 있었습니다. 확장 방향에 가중치를 적용하고, 네 방이 한 덩어리로 붙는 배치를 제한해 맵 형태와 갈림길을 다양화했습니다.',64,260,442,144,20,C.text,false);
 txt(s,'적용한 규칙',64,431,442,24,15,C.gold,true);
 txt(s,'확장 방향별 가중치   ·   2×2 배치 제한',64,461,442,35,19,C.navy,true);
 txt(s,'기본 랜덤 확장',620,209,250,28,17,C.muted,true,'center');
 txt(s,'규칙 적용 후',930,209,260,28,17,C.muted,true,'center');
 // Before: compact 3x3 cluster representing square-like layouts.
 const bx=654,by=283,gap=59,sz=44;
 for(let row=0;row<3;row++)for(let col=0;col<3;col++)mapNode(s,bx+col*gap,by+row*gap,'','room',sz);
 mapNode(s,bx+gap,by+gap,'S','start',sz);
 // After: branching connected route; connectors are behind editable room nodes.
 const pts={start:[914,400],a:[980,400],b:[1046,345],c:[1046,455],d:[1112,400],boss:[1178,400],shop:[1112,510]};
 connector(s,940,426,1006,426);connector(s,1006,426,1072,426);connector(s,1072,371,1072,481);connector(s,1072,426,1138,426);connector(s,1138,426,1204,426);connector(s,1138,452,1138,510);
 mapNode(s,...pts.start,'S','start',52);mapNode(s,...pts.a,'','room',52);mapNode(s,...pts.b,'','room',52);mapNode(s,...pts.c,'','room',52);mapNode(s,...pts.d,'','room',52);mapNode(s,...pts.boss,'B','boss',52);mapNode(s,...pts.shop,'R','special',52);
 txt(s,'S 시작   B 보스   R 보상',905,574,300,25,13,C.muted,false,'center');
}
// Slide 2: room combat and reward loop.
{
 const s=p.slides.add();s.background.fill=C.bg;header(s,'COMBAT & REWARD',2);
 mainTitle(s,'전투와 보상 진행 구조','방 진입부터 클리어까지 전투 흐름을 하나로 연결');
 label(s,'방 단위 전투 흐름',64,208,1152);
 const steps=[['01','방 진입'],['02','몬스터 등장'],['03','출구 잠금'],['04','전멸 확인'],['05','출구 개방'],['06','보상 획득']];
 const sx=64,sw=166,g=29,y=276;
 for(let i=0;i<steps.length;i++){
  const x=sx+i*(sw+g); const active=i===2||i===5;
  rect(s,x,y,sw,82,active?C.navy:C.white,active?C.navy:C.line,2);
  txt(s,steps[i][1],x+12,y+10,sw-24,62,17,active?C.white:C.text,true,'center');
  if(i<steps.length-1){txt(s,'›',x+sw+4,y+24,g-6,32,24,C.gold,true,'center')}
 }
 label(s,'몬스터 구성',64,408,520);
 txt(s,'방마다 적의 종류와 수가 달라져 전투 조합이 바뀝니다.',64,452,520,54,18,C.text,false);
 label(s,'증원 웨이브',640,408,576);
 txt(s,'조건 충족  →  등장 위치 예고  →  추가 몬스터 등장',640,452,576,54,18,C.text,false);
}
// Slide 3: exploration information and stage progression.
{
 const s=p.slides.add();s.background.fill=C.bg;header(s,'EXPLORATION & STAGE FLOW',3);
 mainTitle(s,'탐험 정보와 다음 스테이지','미니맵으로 위치를 확인하고 보스 클리어 후 다음 구역으로 진행');
 label(s,'탐험 미니맵',64,208,470);
 // Editable minimap route diagram with visit and destination states.
 const cells=[
  [185,285,'S','start'],[251,285,'','visited'],[317,285,'','visited'],[383,285,'B','boss'],
  [251,351,'','visited'],[251,417,'','current'],[317,417,'','unknown'],[383,417,'R','special'],
 ];
 const edgePairs=[[0,1],[1,2],[2,3],[1,4],[4,5],[5,6],[6,7]];
 for(const [a,b] of edgePairs){const A=cells[a],B=cells[b];connector(s,A[0]+26,A[1]+26,B[0]+26,B[1]+26,C.line)}
 for(const [x,y,t,type] of cells){const fill=type==='start'?C.gold:type==='boss'?C.navy:type==='current'?C.blue:type==='visited'?C.pale:type==='special'?C.goldPale:C.white;const fg=['start','boss','current'].includes(type)?C.white:C.text;mapNode(s,x,y,t,type==='visited'||type==='unknown'?'room':type,52);if(type==='visited')rect(s,x,y,52,52,C.pale,C.line,2);if(type==='current')rect(s,x,y,52,52,C.blue,C.blue,2);if(type==='unknown')rect(s,x,y,52,52,C.white,C.line,2);if(t)txt(s,t,x+4,y+4,44,44,14,fg,true,'center')}
 txt(s,'시작',64,495,80,25,14,C.gold,true);txt(s,'방문',148,495,80,25,14,C.muted,true);txt(s,'현재',232,495,80,25,14,C.blue,true);txt(s,'보스 목표',316,495,120,25,14,C.navy,true);
 label(s,'스테이지 진행',620,208,596);
 const flow=[['보스방 클리어',620],['출구 활성화',815],['새 맵 생성',1010]];
 for(let i=0;i<flow.length;i++){
  const [t,x]=flow[i];rect(s,x,286,166,68,i===2?C.navy:C.white,i===2?C.navy:C.line,2);txt(s,t,x+7,300,152,40,17,i===2?C.white:C.text,true,'center');
  if(i<2)txt(s,'›',x+168,300,27,38,23,C.gold,true,'center')
 }
 txt(s,'보상·상점 이동 뒤에도 방문 기록과 복귀 위치를 유지합니다.',620,390,596,55,18,C.text,false);
 txt(s,'새 스테이지 생성 후 플레이어를 시작 방으로 이동합니다.',620,460,596,55,18,C.text,false);
}
await fs.mkdir(build,{recursive:true});
for(let i=0;i<p.slides.items.length;i++){const blob=await p.slides.items[i].export({format:'png',scale:1});await fs.writeFile(`${build}/diagram-slide-${i+1}.png`,new Uint8Array(await blob.arrayBuffer()))}
const candidate=`${build}/candidate-diagram.pptx`;await (await PresentationFile.exportPptx(p)).save(candidate);
const {finalizePresentation}=await import(pathToFileURL(path.join(skill,'container_tools/artifact_tool_utils.mjs')).href);
const result=await finalizePresentation({explicitTotalSlideCount:3,workspaceDir:root,candidatePath:candidate,finalPath,pythonExecutable:'C:/Program Files/WindowsApps/OpenAI.CodexPrimaryRuntime.v26-1112-737-0_26.1112.737.0_x64__3k8sg7r9htsxt/dependencies/python/python.exe',integrityValidatorPath:path.join(skill,'container_tools/inspect_presentation_package_integrity.py'),layoutValidatorPath:path.join(skill,'container_tools/inspect_presentation_layout_geometry.py'),layoutArgs:['--expected-slide-size-emu','9144000,5143500','--validate-bullet-geometry','--validate-heading-fit'],fontPolicy:{basis:'design',families:['Arial']},verifyArtifactToolImport:true,receiptPath:`${build}/validation-diagram.json`});
console.log(JSON.stringify(result));
