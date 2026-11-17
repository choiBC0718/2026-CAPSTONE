import fs from 'node:fs/promises';
import { createHash } from 'node:crypto';
import path from 'node:path';
import { pathToFileURL } from 'node:url';
import { Presentation, PresentationFile } from 'file:///C:/Program%20Files/WindowsApps/OpenAI.CodexPrimaryRuntime.v26-1112-737-0_26.1112.737.0_x64__3k8sg7r9htsxt/dependencies/node/node_modules/@oai/artifact-tool/dist/artifact_tool.mjs';

const workspaceDir = 'C:/Project/2026-CAPSTONE';
const buildDir = `${workspaceDir}/output/poster-build`;
const finalPath = `${workspaceDir}/Reports/P_CAP_졸업작품전시_3장.pptx`;
const SKILL_DIR = 'C:/Users/MinSeok/.codex/plugins/cache/openai-primary-runtime/presentations/26.1112.11217/skills/presentations';
const W=960, H=540, S=0.75;
const C={paper:'#FFF2CC', ink:'#111111', brown:'#5F5543', gold:'#C68D1C', pale:'#FFE699', white:'#FFFFFF', muted:'#8A7A5D'};
const FONT='Arial';
const p=Presentation.create({slideSize:{width:W,height:H}});

function rect(slide,x,y,w,h,fill,stroke='none',sw=0){
  return slide.shapes.add({geometry:'rect',position:{left:x*S,top:y*S,width:w*S,height:h*S},fill,
    line:{style:'solid',fill:stroke,width:sw}});
}
function text(slide,txt,x,y,w,h,size,color=C.ink,bold=false,opts={}){
  const s=slide.shapes.add({geometry:'textbox',position:{left:x*S,top:y*S,width:w*S,height:h*S},fill:'none',line:{style:'solid',fill:'none',width:0}});
  s.text=txt;
  s.text.style={fontFamily:FONT,fontSize:size*S,color,bold,verticalAlignment:'middle',...opts};
  return s;
}
function header(slide,tag='GRADUATION PROJECT'){
  text(slide,`P_CAP  |  ${tag}`,64,24,850,28,13,C.brown,true);
  rect(slide,64,61,1152,2,C.gold);
}
function footer(slide,n){
  rect(slide,64,670,1152,1,'#D9CBA5');
  text(slide,'202104320 임민석  ·  P_CAP 졸업작품 전시회',64,680,850,22,11,C.brown,true);
  text(slide,`${String(n).padStart(2,'0')} / 03`,1090,680,126,22,11,C.brown,true,{textAlign:'right'});
}
function section(slide,label,x,y,w){
  text(slide,label,x,y,w,30,17,C.gold,true);
  rect(slide,x,y+35,w,1,'#D9CBA5');
}
function room(slide,x,y,label,fill,txtColor=C.ink,w=126,h=70){
  rect(slide,x,y,w,h,fill,C.gold,2);
  text(slide,label,x+5,y+7,w-10,h-14,18,txtColor,true,{textAlign:'center'});
}
function connector(slide,x,y,w,h=4){rect(slide,x,y,w,h,C.gold);}

// Slide 1: title plus the central procedural-generation challenge.
{
  const s=p.slides.add(); s.background.fill=C.paper; header(s,'GRADUATION PROJECT');
  text(s,'P_CAP  ·  GRADUATION PROJECT',66,93,560,30,16,C.gold,true);
  text(s,'절차적 맵 생성 기반 로그라이크 액션 게임',66,128,1080,54,34,C.ink,true);
  text(s,'무작위 맵 생성에서 전투와 스테이지 진행까지',66,188,1080,34,18,C.brown,false);
  section(s,'맵 알고리즘에서 해결한 과제',66,296,510);
  text(s,'문제',66,347,112,28,16,C.gold,true);
  text(s,'방을 무작위로 늘리면 연결이 끊기거나 2×2로 뭉치고, 보스·특수방의 위치도 불안정해질 수 있었습니다.',178,345,398,76,17,C.ink,false);
  text(s,'해결',66,442,112,28,16,C.gold,true);
  text(s,'가중치 기반 확장과 2×2 배치 제한을 적용하고, BFS 거리로 보스방을 지정했습니다. 막다른 방은 보상·상점 후보로 분류했습니다.',178,440,398,105,17,C.ink,false);
  text(s,'결과',66,565,112,28,16,C.gold,true);
  text(s,'맵 모양은 달라져도 연결 경로와 방의 역할이 유지됩니다.',178,563,398,54,17,C.brown,true);
  section(s,'생성 규칙 예시',640,296,576);
  text(s,'시작점에서 확장하고, 거리와 연결 상태로 방의 역할을 정합니다',640,339,576,44,16,C.brown,false);
  // Editable map diagram: generated room graph with labeled room roles.
  connector(s,758,462,94,4); connector(s,884,462,94,4); connector(s,1010,462,94,4);
  connector(s,821,496,4,67); connector(s,947,496,4,67);
  room(s,632,428,'시작',C.gold,C.white);
  room(s,758,428,'일반',C.pale);
  room(s,884,428,'일반',C.pale);
  room(s,1010,428,'보스',C.gold,C.white);
  room(s,758,563,'상점',C.white);
  room(s,884,563,'보상',C.white);
  text(s,'가중치 확장',642,626,190,26,13,C.brown,true);
  text(s,'BFS 거리 분류',846,626,190,26,13,C.brown,true);
  text(s,'막다른 방 판정',1040,626,176,26,13,C.brown,true);
  footer(s,1);
}

// Slide 2: from room generation to combat and reward loop.
{
  const s=p.slides.add(); s.background.fill=C.paper; header(s,'MAP & COMBAT SYSTEM');
  text(s,'생성된 방을 실제 전투 흐름으로 연결',64,91,1120,50,34,C.ink,true);
  text(s,'몬스터 배치, 전투 상태, 포탈과 보상이 따로 놀지 않도록 방 단위 진행을 구성',64,146,1120,35,18,C.brown,false);
  section(s,'전투 진행 흐름',64,212,1152);
  const steps=[['01','방 진입'],['02','점수 예산 스폰'],['03','포탈 잠금'],['04','전멸 판정 · 보상']];
  const xs=[64,356,648,940];
  for(let i=0;i<steps.length;i++){
    rect(s,xs[i],266,252,72,i===3?C.gold:C.white,C.gold,2);
    text(s,steps[i][1],xs[i]+14,276,224,32,17,i===3?C.white:C.ink,true,{textAlign:'center'});
    if(i<steps.length-1){text(s,'→',xs[i]+258,285,32,25,21,C.gold,true,{textAlign:'center'});}
  }
  section(s,'문제',64,377,510);
  text(s,'스폰 규칙이 방 로직에 섞이면 수정과 밸런스 조정이 어려워집니다. 전투 중 출구가 열려 있으면 클리어 전에 방을 벗어날 수도 있습니다.',64,421,510,90,18,C.ink,false);
  section(s,'해결',640,377,576);
  text(s,'RoomMonsterSpawnerComponent로 스폰 책임을 분리하고, DataAsset에서 몬스터 비용·가중치·최대 수량을 조정했습니다. 방 점수 예산으로 조합을 만들고, 전투 중 문을 잠근 뒤 전멸 시 다시 엽니다.',640,421,576,108,18,C.ink,false);
  rect(s,64,554,1152,84,C.pale);
  text(s,'결과',84,572,90,28,16,C.gold,true);
  text(s,'방마다 다른 몬스터 조합 · 명확한 클리어 판정 · 방 보상 선택과 처치 재화 지급',182,567,1005,42,19,C.ink,true);
  footer(s,2);
}

// Slide 3: exploration state, wave telegraph, and stage transition.
{
  const s=p.slides.add(); s.background.fill=C.paper; header(s,'EXPLORATION & STAGE FLOW');
  text(s,'탐험 정보를 유지하고 다음 스테이지로',64,91,1120,50,34,C.ink,true);
  text(s,'미니맵부터 증원 예고와 보스방 출구까지 플레이의 연속성을 보완',64,146,1120,35,18,C.brown,false);
  section(s,'탐험 상태',64,212,500);
  text(s,'미방문 방과 현재 목표가 잘 보이지 않고, 특수방을 다녀온 뒤에도 현재 위치와 방문 기록을 복원해야 했습니다.',64,253,500,67,17,C.ink,false);
  // Small editable map key.
  room(s,86,354,'시작',C.brown,C.white,100,58);
  room(s,217,354,'방문',C.white,C.ink,100,58);
  room(s,348,354,'현재',C.gold,C.white,100,58);
  room(s,479,354,'목표',C.pale,C.ink,80,58);
  text(s,'방 종류와 방문 상태를 색으로 표시하고, 현재 방·보스 목표를 구분',64,423,500,50,16,C.brown,true);
  text(s,'상점·보상방 이동 전 Seed·복귀 좌표·방문 기록을 저장하고 복원',64,486,500,54,16,C.brown,false);
  section(s,'전투와 스테이지 전환',640,212,576);
  text(s,'증원 위치를 미리 보여주고, 보스방 클리어 뒤 새 맵으로 이어지도록 구성',640,253,576,52,17,C.ink,false);
  const rows=[
    {y:333,items:['조건 충족','위치 예고 VFX','증원 스폰']},
    {y:435,items:['보스 전멸','출구 활성화','새 맵 생성']},
  ];
  for(const row of rows){
    for(let i=0;i<row.items.length;i++){
      const x=640+i*194;
      rect(s,x,row.y,166,58,i===2?C.gold:C.white,C.gold,2);
      text(s,row.items[i],x+8,row.y+12,150,34,16,i===2?C.white:C.ink,true,{textAlign:'center'});
      if(i<2)text(s,'→',x+167,row.y+14,27,28,19,C.gold,true,{textAlign:'center'});
    }
  }
  text(s,'생존 수·전투 시간 조건으로 웨이브를 예약하고, 실제 스폰 위치를 먼저 알립니다.',640,405,576,24,14,C.brown,false);
  text(s,'로딩 UI 중 맵을 재생성하고 플레이어를 새 시작방으로 이동합니다.',640,507,576,24,14,C.brown,false);
  rect(s,64,567,1152,68,C.pale);
  text(s,'결과',84,585,90,28,16,C.gold,true);
  text(s,'방문 상태와 목표를 읽을 수 있고, 전투·보스 클리어·다음 스테이지가 한 흐름으로 이어집니다.',182,580,1005,40,18,C.ink,true);
  footer(s,3);
}

await fs.mkdir(buildDir,{recursive:true});
await fs.mkdir(`${workspaceDir}/output`,{recursive:true});
for (let i=0;i<p.slides.items.length;i++){
  const blob=await p.slides.items[i].export({format:'png',scale:1});
  await fs.writeFile(`${buildDir}/slide-${i+1}.png`,new Uint8Array(await blob.arrayBuffer()));
}
const candidate=`${buildDir}/candidate.pptx`;
await (await PresentationFile.exportPptx(p)).save(candidate);
const {finalizePresentation}=await import(pathToFileURL(path.join(SKILL_DIR,'container_tools/artifact_tool_utils.mjs')).href);
const final=await finalizePresentation({
  explicitTotalSlideCount:3,
  workspaceDir,
  candidatePath:candidate,
  finalPath,
  pythonExecutable:'C:/Program Files/WindowsApps/OpenAI.CodexPrimaryRuntime.v26-1112-737-0_26.1112.737.0_x64__3k8sg7r9htsxt/dependencies/python/python.exe',
  integrityValidatorPath:path.join(SKILL_DIR,'container_tools/inspect_presentation_package_integrity.py'),
  layoutValidatorPath:path.join(SKILL_DIR,'container_tools/inspect_presentation_layout_geometry.py'),
  layoutArgs:['--expected-slide-size-emu','9144000,5143500','--validate-bullet-geometry','--validate-heading-fit'],
  fontPolicy:{basis:'design',families:['Arial']},
  verifyArtifactToolImport:true,
  receiptPath:`${buildDir}/validation.json`
});
console.log(JSON.stringify(final));






