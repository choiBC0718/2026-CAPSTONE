import fs from 'node:fs/promises';
import path from 'node:path';
import { pathToFileURL } from 'node:url';
import { Presentation, PresentationFile } from 'file:///C:/Program%20Files/WindowsApps/OpenAI.CodexPrimaryRuntime.v26-1112-737-0_26.1112.737.0_x64__3k8sg7r9htsxt/dependencies/node/node_modules/@oai/artifact-tool/dist/artifact_tool.mjs';
const root='C:/Project/2026-CAPSTONE';
const build=`${root}/output/poster-build`;
const finalPath=`${root}/Reports/P_CAP_졸업작품전시_3장_쉬운설명.pptx`;
const skill='C:/Users/MinSeok/.codex/plugins/cache/openai-primary-runtime/presentations/26.1112.11217/skills/presentations';
const S=.75, W=960, H=540, FONT='Arial';
const C={paper:'#FFF2CC',ink:'#111111',brown:'#5F5543',gold:'#C68D1C',pale:'#FFE699',white:'#FFFFFF',rule:'#D9CBA5'};
const p=Presentation.create({slideSize:{width:W,height:H}});
function rect(slide,x,y,w,h,fill,stroke='none',sw=0){return slide.shapes.add({geometry:'rect',position:{left:x*S,top:y*S,width:w*S,height:h*S},fill,line:{style:'solid',fill:stroke,width:sw}})}
function txt(slide,value,x,y,w,h,size,color=C.ink,bold=false,align='left'){
 const s=slide.shapes.add({geometry:'textbox',position:{left:x*S,top:y*S,width:w*S,height:h*S},fill:'none',line:{style:'solid',fill:'none',width:0}});
 s.text=value;s.text.style={fontFamily:FONT,fontSize:size*S,color,bold,verticalAlignment:'middle',textAlign:align};return s;
}
function header(slide,tag){txt(slide,`졸업작품 전시회  |  ${tag}`,64,24,920,28,13,C.brown,true);rect(slide,64,61,1152,2,C.gold)}
function footer(slide,n){rect(slide,64,670,1152,1,C.rule);txt(slide,'202104320 임민석  ·  졸업작품 전시회',64,680,850,22,11,C.brown,true);txt(slide,`${String(n).padStart(2,'0')} / 03`,1090,680,126,22,11,C.brown,true,'right')}
function title(slide,main,sub){txt(slide,main,64,89,1152,54,34,C.ink,true);txt(slide,sub,64,146,1152,34,19,C.brown,false)}
function point(slide,label,copy,y,h=82){txt(slide,label,64,y,360,27,18,C.gold,true);txt(slide,copy,64,y+32,360,h,21,C.ink,false)}
async function addShot(slide,file,x,y,w,h,alt,crop){
 const blob=new Uint8Array(await fs.readFile(file));
 slide.images.add({blob,contentType:'image/png',alt,fit:'cover',position:{left:x*S,top:y*S,width:w*S,height:h*S},...(crop?{crop}:{})});
 rect(slide,x,y,w,h,'none',C.gold,2);
}
const imgMap=`${build}/refs/10주차_주간업무보고서_202104320임민석_1.png`;
const imgBoss=`${build}/refs/10주차_주간업무보고서_202104320임민석_3.png`;
const imgFight=`${build}/refs/7주차_주간업무보고서_202104320_임민석_1.png`;
// 1: work title and the map-generation challenge
{
 const s=p.slides.add();s.background.fill=C.paper;header(s,'맵 만들기');
 title(s,'로그라이크 액션 게임','맵이 달라져도 길은 이어지는 던전 탐험');
 point(s,'어려웠던 점','방을 무작위로 놓으면 길이 끊기거나 한쪽에 몰렸습니다.',218,78);
 point(s,'해결','방을 서로 이어 만들고, 가장 먼 곳은 보스방으로 정했습니다. 끝방에는 보상이나 상점을 배치했습니다.',337,102);
 point(s,'결과','맵은 달라져도 길과 방의 역할은 유지됩니다.',482,75);
 await addShot(s,imgMap,470,218,746,340,'게임 화면: 만들어진 방과 미니맵');
 txt(s,'방을 이어 만든 던전과 화면 오른쪽 위 미니맵',470,568,746,34,15,C.brown,false);
 footer(s,1);
}
// 2: plain-language combat loop
{
 const s=p.slides.add();s.background.fill=C.paper;header(s,'전투와 보상');
 title(s,'적을 물리치면 다음 방이 열립니다','방마다 다른 전투와 보상을 만날 수 있도록 구성했습니다');
 point(s,'어려웠던 점','싸우는 중에 출구로 나가거나, 매번 비슷한 적만 만나면 재미가 떨어질 수 있습니다.',218,92);
 point(s,'해결','방마다 다른 적을 배치하고, 싸우는 동안에는 출구를 닫았습니다. 모두 물리치면 출구가 열리고 보상을 받습니다.',352,108);
 await addShot(s,imgFight,470,218,746,340,'전투 화면: 몬스터와 마법진이 보이는 장면',{left:0.08,top:0.12,right:0.08,bottom:0.03});
 rect(s,470,568,746,48,C.pale);
 txt(s,'적 등장  →  출구 닫힘  →  모두 물리치면 보상',486,576,714,30,17,C.ink,true,'center');
 footer(s,2);
}
// 3: exploration and stage progression
{
 const s=p.slides.add();s.background.fill=C.paper;header(s,'탐험과 다음 단계');
 title(s,'미니맵으로 길과 목표를 찾습니다','보스를 물리치면 출구가 열리고 다음 구역으로 이어집니다');
 point(s,'어려웠던 점','맵을 돌아다닐 때 어디를 가봤는지, 보스가 어디 있는지 알기 어려웠습니다.',218,90);
 point(s,'해결','방문한 곳과 현재 위치, 보스 목표를 미니맵에 다른 색으로 표시했습니다. 보스를 물리치면 다음 단계로 이동합니다.',349,104);
 point(s,'결과','갈 길과 목표가 잘 보이고, 탐험이 다음 구역까지 이어집니다.',496,75);
 await addShot(s,imgBoss,470,218,746,340,'게임 화면: 미니맵에 보스 목표가 표시된 장면');
 txt(s,'보스 목표와 현재 위치를 확인할 수 있는 미니맵',470,568,746,34,15,C.brown,false);
 footer(s,3);
}
await fs.mkdir(build,{recursive:true});
for(let i=0;i<p.slides.items.length;i++){const b=await p.slides.items[i].export({format:'png',scale:1});await fs.writeFile(`${build}/simple-slide-${i+1}.png`,new Uint8Array(await b.arrayBuffer()))}
const candidate=`${build}/candidate-simple.pptx`;
await (await PresentationFile.exportPptx(p)).save(candidate);
const {finalizePresentation}=await import(pathToFileURL(path.join(skill,'container_tools/artifact_tool_utils.mjs')).href);
const result=await finalizePresentation({explicitTotalSlideCount:3,workspaceDir:root,candidatePath:candidate,finalPath,pythonExecutable:'C:/Program Files/WindowsApps/OpenAI.CodexPrimaryRuntime.v26-1112-737-0_26.1112.737.0_x64__3k8sg7r9htsxt/dependencies/python/python.exe',integrityValidatorPath:path.join(skill,'container_tools/inspect_presentation_package_integrity.py'),layoutValidatorPath:path.join(skill,'container_tools/inspect_presentation_layout_geometry.py'),layoutArgs:['--expected-slide-size-emu','9144000,5143500','--validate-bullet-geometry','--validate-heading-fit'],fontPolicy:{basis:'design',families:['Arial']},verifyArtifactToolImport:true,receiptPath:`${build}/validation-simple.json`});
console.log(JSON.stringify(result));
