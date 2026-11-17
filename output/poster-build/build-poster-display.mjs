import fs from 'node:fs/promises';
import path from 'node:path';
import { pathToFileURL } from 'node:url';
import { Presentation, PresentationFile } from 'file:///C:/Program%20Files/WindowsApps/OpenAI.CodexPrimaryRuntime.v26-1112-737-0_26.1112.737.0_x64__3k8sg7r9htsxt/dependencies/node/node_modules/@oai/artifact-tool/dist/artifact_tool.mjs';
const root='C:/Project/2026-CAPSTONE', build=`${root}/output/poster-build`;
const finalPath=`${root}/Reports/P_CAP_졸업작품전시_3장_설정흐름.pptx`;
const skill='C:/Users/MinSeok/.codex/plugins/cache/openai-primary-runtime/presentations/26.1112.11217/skills/presentations';
const S=.75, W=960, H=540, FONT='Arial';
const C={paper:'#FFF2CC',ink:'#111111',brown:'#5F5543',gold:'#C68D1C',pale:'#FFE699',white:'#FFFFFF',rule:'#D9CBA5'};
const p=Presentation.create({slideSize:{width:W,height:H}});
function rect(s,x,y,w,h,fill,stroke='none',sw=0){return s.shapes.add({geometry:'rect',position:{left:x*S,top:y*S,width:w*S,height:h*S},fill,line:{style:'solid',fill:stroke,width:sw}})}
function txt(s,v,x,y,w,h,size,color=C.ink,bold=false,align='left'){
 const sh=s.shapes.add({geometry:'textbox',position:{left:x*S,top:y*S,width:w*S,height:h*S},fill:'none',line:{style:'solid',fill:'none',width:0}});
 sh.text=v;sh.text.style={fontFamily:FONT,fontSize:size*S,color,bold,verticalAlignment:'middle',textAlign:align};return sh;
}
function header(s,topic){txt(s,`졸업작품 전시회  |  ${topic}`,64,24,960,28,13,C.brown,true);rect(s,64,61,1152,2,C.gold)}
function footer(s,n){rect(s,64,670,1152,1,C.rule);txt(s,'202104320 임민석  ·  졸업작품 전시회',64,680,850,22,11,C.brown,true);txt(s,`${String(n).padStart(2,'0')} / 03`,1090,680,126,22,11,C.brown,true,'right')}
function heading(s,title,sub){txt(s,title,64,88,1152,54,36,C.ink,true);txt(s,sub,64,145,1152,36,20,C.brown,false)}
async function shot(s,file,alt,crop,box){
 const {x=64,y=218,w=730,h=350}=box||{}; const bytes=new Uint8Array(await fs.readFile(file));
 s.images.add({blob:bytes,contentType:'image/png',alt,fit:'cover',position:{left:x*S,top:y*S,width:w*S,height:h*S},...(crop?{crop}:{})});
 rect(s,x,y,w,h,'none',C.gold,2);
}
function feature(s,n,label,desc,y){
 txt(s,n,830,y,42,24,14,C.gold,true);
 txt(s,label,870,y-2,346,31,20,C.ink,true);
 txt(s,desc,870,y+34,346,58,18,C.brown,false);
 if(n!=='03')rect(s,830,y+104,386,1,C.rule);
}
function flowStep(s,n,label,desc,y){
 txt(s,n,600,y,52,24,14,C.gold,true);
 txt(s,label,660,y-2,556,29,20,C.ink,true);
 txt(s,desc,660,y+30,556,54,18,C.brown,false);
 if(n!=='04')rect(s,600,y+88,616,1,C.rule);
}
const mapImg=`${build}/refs/10주차_주간업무보고서_202104320임민석_1.png`;
const bossImg=`${build}/refs/10주차_주간업무보고서_202104320임민석_3.png`;
const fightImg=`${build}/refs/7주차_주간업무보고서_202104320_임민석_1.png`;
const slides=[
 {topic:'맵 생성 알고리즘',title:'가중치 확장과 BFS로 던전을 구성합니다',sub:'빈 방 후보를 가중치로 선택하고, 시작점부터의 거리를 계산해 보스 방을 정합니다.',img:mapImg,alt:'게임 화면: 생성된 던전과 미니맵',caption:'생성된 던전과 미니맵',flow:[['01','후보 위치 수집','각 방의 상·하·좌·우 빈칸을 새 방 후보로 모읍니다.'],['02','가중치 계산','주변 방 개수와 확장 기준 방의 연결 수에 따라 후보별 점수를 조정합니다.'],['03','확률 기반 확장','점수가 높은 후보를 더 자주 선택합니다. 2×2로 뭉치는 후보는 제외해 맵 형태를 분산합니다.'],['04','BFS로 보스 지정','시작 방에서 큐에 인접 방을 차례로 넣어 거리를 계산하고, 가장 먼 방을 보스 방으로 정합니다.']]},
 {topic:'몬스터 배치 설정',title:'방 예산에 맞춰 몬스터를 배치합니다',sub:'방 점수 예산과 몬스터별 비용으로 다양한 전투를 구성합니다.',img:fightImg,alt:'게임 화면: 몬스터와 마법진이 있는 전투 장면',crop:{left:0.08,top:0.12,right:0.08,bottom:0.03},caption:'게임 화면  ·  방마다 다른 몬스터 조합으로 전투',features:[['01','방 점수 예산','예시: 한 방에 사용할 수 있는 예산은 20점'],['02','몬스터별 비용','약한 적 2점 · 중간 적 3점 · 강한 적 5점'],['03','배치 흐름','예산 설정 → 비용에 맞춰 선택 → 방에 배치']]},
 {topic:'스테이지 구성',title:'스테이지마다 방 수와 적 구성을 설정합니다',sub:'보스를 물리치면 출구가 열리고 다음 구역으로 이어집니다.',img:bossImg,alt:'게임 화면: 미니맵에 현재 위치와 보스 목표가 표시된 장면',caption:'게임 화면  ·  현재 위치와 보스 목표를 확인하는 미니맵',features:[['01','스테이지 진행','스테이지 1 → 2 → 3 순서로 진행'],['02','방 개수','기본 10개 · 스테이지별 조정 가능'],['03','다음 구역','보스 클리어 → 출구 개방 → 새 맵']]}
];
for(let i=0;i<slides.length;i++){
 const d=slides[i],s=p.slides.add();s.background.fill=C.paper;header(s,d.topic);heading(s,d.title,d.sub);
 if(i===0){
  await shot(s,d.img,d.alt,d.crop,{x:64,y:245,w:500,h:310});
  txt(s,d.caption,64,565,500,30,14,C.brown,false);
  d.flow.forEach((step,j)=>flowStep(s,...step,218+j*108));
 } else {
  await shot(s,d.img,d.alt,d.crop);
  txt(s,d.caption,64,581,730,32,15,C.brown,false);
  feature(s,...d.features[0],227);feature(s,...d.features[1],363);feature(s,...d.features[2],499);
 }
 footer(s,i+1);
}
await fs.mkdir(build,{recursive:true});
for(let i=0;i<p.slides.items.length;i++){const blob=await p.slides.items[i].export({format:'png',scale:1});await fs.writeFile(`${build}/display-slide-${i+1}.png`,new Uint8Array(await blob.arrayBuffer()))}
const candidate=`${build}/candidate-display.pptx`;await (await PresentationFile.exportPptx(p)).save(candidate);
const {finalizePresentation}=await import(pathToFileURL(path.join(skill,'container_tools/artifact_tool_utils.mjs')).href);
const result=await finalizePresentation({explicitTotalSlideCount:3,workspaceDir:root,candidatePath:candidate,finalPath,pythonExecutable:'C:/Program Files/WindowsApps/OpenAI.CodexPrimaryRuntime.v26-1112-737-0_26.1112.737.0_x64__3k8sg7r9htsxt/dependencies/python/python.exe',integrityValidatorPath:path.join(skill,'container_tools/inspect_presentation_package_integrity.py'),layoutValidatorPath:path.join(skill,'container_tools/inspect_presentation_layout_geometry.py'),layoutArgs:['--expected-slide-size-emu','9144000,5143500','--validate-bullet-geometry','--validate-heading-fit'],fontPolicy:{basis:'design',families:['Arial']},verifyArtifactToolImport:true,receiptPath:`${build}/validation-display.json`});
console.log(JSON.stringify(result));
