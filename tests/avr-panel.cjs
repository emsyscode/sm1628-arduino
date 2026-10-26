const fs=require('node:fs'),assert=require('node:assert/strict');
module.exports=avr=>function machine(file, panel=false, held=false) {
 const flash=new Uint8Array(32768);
 for(const line of fs.readFileSync(file,'utf8').trim().split(/\r?\n/)){
  const r=Buffer.from(line.slice(1),'hex');
  assert.equal([...r].reduce((a,b)=>a+b,0)%256,0);
  if(r[3]===0)flash.set(r.subarray(4,4+r[0]),r.readUInt16BE(1));
 }
 const cpu=new avr.CPU(new Uint16Array(flash.buffer));
 for(const c of [avr.timer0Config,avr.timer1Config,avr.timer2Config])new avr.AVRTimer(cpu,c);
 const b=new avr.AVRIOPort(cpu,avr.portBConfig);
 const portC=new avr.AVRIOPort(cpu,avr.portCConfig);
 const portD=new avr.AVRIOPort(cpu,avr.portDConfig);portD.setPin(4,!held);
 const uart=new avr.AVRUSART(cpu,avr.usart0Config,16e6);
 const adc=new avr.AVRADC(cpu,avr.adcConfig);adc.channelValues[0]=2.716;adc.channelValues[1]=2.709;
 const result={output:'',frames:[],buzzerEdges:[],cpu,adc,portC,portD,run,send};let bits=[],active=false;
 uart.onByteTransmit=v=>result.output+=String.fromCharCode(v);
 if(panel)b.addListener((v,o)=>{
  if((v&8)!==(o&8))result.buzzerEdges.push({ms:cpu.cycles/16000,on:!!(v&8)});
  if((o&2)&&!(v&2)){bits=[];active=true;}
  if(!(v&2)&&!(o&1)&&(v&1)){if(!active){active=true;bits=[];}bits.push((cpu.data[0x2B]>>7)&1);}
  if(!(o&2)&&(v&2)&&active){
   assert.equal(bits.length%8,0);const bytes=[];
   for(let i=0;i<bits.length;i+=8)bytes.push(bits.slice(i,i+8).reduce((a,x,k)=>a|(x<<k),0));
   if(bytes[0]===192 && bytes.length===15)result.frames.push({ms:cpu.cycles/16000,data:bytes.slice(1)});
   active=false;
  }
 });
 function run(ms){const end=cpu.cycles+ms*16000;while(cpu.cycles<end){avr.avrInstruction(cpu);cpu.tick();}}
 function send(text,wait=100){for(const c of text+'\n'){assert(uart.writeByte(c.charCodeAt(0)));run(.15);}run(wait);}
 return result;
};

