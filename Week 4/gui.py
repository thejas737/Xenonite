import ctypes, tkinter as tk
from tkinter import ttk, messagebox
class App:
 def __init__(self,root):
  self.root=root;root.title("Deadlock Simulation System | OS + DSA PBL");root.geometry("1160x800");root.configure(bg="#eef2f7")
  self.lib=ctypes.CDLL("./libdeadlock.so");I=ctypes.POINTER(ctypes.c_int)
  self.lib.banker_safe.argtypes=[ctypes.c_int,ctypes.c_int,I,I,I,I];self.lib.banker_safe.restype=ctypes.c_int
  self.lib.detect_deadlock.argtypes=[ctypes.c_int,ctypes.c_int,I,I,I,I]
  self.lib.request_resources.argtypes=[ctypes.c_int,ctypes.c_int,ctypes.c_int,I,I,I,I,I];self.lib.request_resources.restype=ctypes.c_int
  self.lib.release_process_resources.argtypes=[ctypes.c_int,ctypes.c_int,ctypes.c_int,I,I,I]
  self.p,self.r=5,3;self.av=[3,3,2];self.al=[[0,1,0],[2,0,0],[3,0,2],[2,1,1],[0,0,2]]
  self.mx=[[7,5,3],[3,2,2],[9,0,2],[2,2,2],[4,3,3]];self.req=[[0]*3 for _ in range(5)]
  s=ttk.Style();s.theme_use("clam");s.configure("TFrame",background="#eef2f7");s.configure("Title.TLabel",font=("Segoe UI",20,"bold"),background="#eef2f7",foreground="#17243a")
  s.configure("TLabel",background="#eef2f7");s.configure("Treeview",rowheight=25)
  head=ttk.Frame(root,padding=15);head.pack(fill="x");ttk.Label(head,text="DEADLOCK SIMULATION SYSTEM",style="Title.TLabel").pack(anchor="w")
  ttk.Label(head,text="Resource allocation • Banker's safety • Detection • Recovery").pack(anchor="w")
  body=ttk.Frame(root,padding=12);body.pack(fill="both",expand=True);left=ttk.Frame(body);left.pack(side="left",fill="y",padx=(0,10));right=ttk.Frame(body);right.pack(side="left",fill="both",expand=True)
  box=ttk.LabelFrame(left,text="System dimensions",padding=8);box.pack(fill="x")
  self.pv=tk.StringVar(value="5");self.rv=tk.StringVar(value="3")
  ttk.Label(box,text="Processes").grid(row=0,column=0);ttk.Entry(box,textvariable=self.pv,width=6).grid(row=0,column=1)
  ttk.Label(box,text="Resources").grid(row=1,column=0);ttk.Entry(box,textvariable=self.rv,width=6).grid(row=1,column=1)
  ttk.Button(box,text="Apply dimensions",command=self.resize).grid(row=2,column=0,columnspan=2,sticky="ew",pady=5)
  edit=ttk.LabelFrame(left,text="Matrices (space-separated)",padding=8);edit.pack(fill="x",pady=8)
  ttk.Label(edit,text="Available vector").pack(anchor="w");self.avt=tk.Entry(edit,width=34);self.avt.pack(fill="x")
  ttk.Label(edit,text="Allocation rows").pack(anchor="w");self.alt=tk.Text(edit,width=34,height=5,font=("Consolas",9));self.alt.pack()
  ttk.Label(edit,text="Maximum rows").pack(anchor="w");self.mxt=tk.Text(edit,width=34,height=5,font=("Consolas",9));self.mxt.pack()
  ttk.Button(edit,text="Load matrices",command=self.load).pack(fill="x",pady=4)
  req=ttk.LabelFrame(left,text="Request resources",padding=8);req.pack(fill="x",pady=4)
  self.pidx=tk.StringVar(value="1");self.reqv=tk.StringVar(value="1 0 2")
  ttk.Label(req,text="Process index (0-based)").pack(anchor="w");ttk.Entry(req,textvariable=self.pidx).pack(fill="x")
  ttk.Label(req,text="Request vector").pack(anchor="w");ttk.Entry(req,textvariable=self.reqv).pack(fill="x")
  ttk.Button(req,text="Submit safe request",command=self.request).pack(fill="x",pady=3)
  ttk.Button(req,text="Queue blocked request",command=self.queue_request).pack(fill="x",pady=3)
  act=ttk.LabelFrame(left,text="Actions",padding=8);act.pack(fill="x",pady=4)
  for label,fn in [("Run Banker's Algorithm",self.banker),("Detect deadlock",self.detect)]:
   ttk.Button(act,text=label,command=fn).pack(fill="x",pady=2)
  self.rec=tk.StringVar(value="0");ttk.Label(act,text="Terminate process index").pack(anchor="w");ttk.Entry(act,textvariable=self.rec).pack(fill="x")
  ttk.Button(act,text="Recover / release resources",command=self.recover).pack(fill="x",pady=3)
  ttk.Button(act,text="Reset sample",command=self.reset).pack(fill="x")
  self.status=tk.StringVar(value="READY");self.detail=tk.StringVar(value="Sample state loaded.")
  status=ttk.LabelFrame(right,text="System status",padding=10);status.pack(fill="x")
  self.status_label=tk.Label(status,textvariable=self.status,bg="white",fg="#16804a",font=("Segoe UI",14,"bold"));self.status_label.pack(anchor="w")
  tk.Label(status,textvariable=self.detail,bg="white",fg="#526174").pack(anchor="w")
  tabs=ttk.Notebook(right);tabs.pack(fill="both",expand=True,pady=8)
  mt=ttk.Frame(tabs,padding=8);gt=ttk.Frame(tabs,padding=8);lt=ttk.Frame(tabs,padding=8)
  tabs.add(mt,text="Matrices & Metrics");tabs.add(gt,text="Resource Allocation Graph");tabs.add(lt,text="Activity Log")
  self.trees={}
  for name in ["Available","Allocation","Maximum","Need"]:
   f=ttk.LabelFrame(mt,text=name,padding=5);f.pack(fill="x",pady=3);t=ttk.Treeview(f,show="headings",height=3);t.pack(fill="x");self.trees[name]=t
  self.metrics=tk.Text(mt,height=4,state="disabled",font=("Consolas",9));self.metrics.pack(fill="x",pady=4)
  self.canvas=tk.Canvas(gt,bg="white",highlightthickness=1);self.canvas.pack(fill="both",expand=True);self.canvas.bind("<Configure>",lambda e:self.graph())
  self.log=tk.Text(lt,state="disabled",font=("Consolas",9));self.log.pack(fill="both",expand=True)
  self.refresh()
 def arr(self,x):return (ctypes.c_int*len(x))(*x)
 def flat(self,m):return self.arr([v for row in m for v in row])
 def note(self,s):self.log.configure(state="normal");self.log.insert("end",s+"\n");self.log.see("end");self.log.configure(state="disabled")
 def resize(self):
  try:
   p,r=int(self.pv.get()),int(self.rv.get())
   if not 1<=p<=20 or not 1<=r<=10:raise ValueError("Use 1-20 processes and 1-10 resource types.")
   self.p,self.r=p,r;self.av=[0]*r;self.al=[[0]*r for _ in range(p)];self.mx=[[0]*r for _ in range(p)];self.req=[[0]*r for _ in range(p)];self.refresh();self.note("Dimensions updated.")
  except Exception as e:messagebox.showerror("Invalid dimensions",str(e))
 def load(self):
  try:
   av=list(map(int,self.avt.get().split()));al=[list(map(int,x.split())) for x in self.alt.get("1.0","end").strip().splitlines()];mx=[list(map(int,x.split())) for x in self.mxt.get("1.0","end").strip().splitlines()]
   if len(av)!=self.r or len(al)!=self.p or len(mx)!=self.p or any(len(x)!=self.r for x in al+mx):raise ValueError("Vector/matrix dimensions do not match the selected process/resource counts.")
   if any(v<0 for v in av+sum(al,[])+sum(mx,[])):raise ValueError("Values must be non-negative.")
   if any(al[i][j]>mx[i][j] for i in range(self.p) for j in range(self.r)):raise ValueError("Allocation cannot exceed Maximum.")
   self.av,self.al,self.mx=av,al,mx;self.req=[[0]*self.r for _ in range(self.p)];self.refresh();self.note("Matrices loaded.")
  except Exception as e:messagebox.showerror("Input error",str(e))
 def refresh(self):
  need=[[self.mx[i][j]-self.al[i][j] for j in range(self.r)] for i in range(self.p)]
  data={"Available":[("System",self.av)],"Allocation":[(f"P{i}",x) for i,x in enumerate(self.al)],"Maximum":[(f"P{i}",x) for i,x in enumerate(self.mx)],"Need":[(f"P{i}",x) for i,x in enumerate(need)]}
  for name,t in self.trees.items():
   t.delete(*t.get_children());cols=["Process"]+[f"R{j}" for j in range(self.r)];t["columns"]=cols
   for c in cols:t.heading(c,text=c);t.column(c,width=60,anchor="center")
   for label,row in data[name]:t.insert("","end",values=[label]+row)
  self.avt.delete(0,"end");self.avt.insert(0," ".join(map(str,self.av)))
  self.alt.delete("1.0","end");self.alt.insert("1.0","\n".join(" ".join(map(str,x)) for x in self.al))
  self.mxt.delete("1.0","end");self.mxt.insert("1.0","\n".join(" ".join(map(str,x)) for x in self.mx))
  self.graph()
 def banker(self):
  av=self.arr(self.av);al=self.flat(self.al);mx=self.flat(self.mx);seq=self.arr([0]*self.p);safe=self.lib.banker_safe(self.p,self.r,av,al,mx,seq)
  if safe:
   order=" -> ".join("P"+str(seq[i]) for i in range(self.p));self.setstatus("SAFE STATE","#16804a","Safe sequence: "+order);self.note("Banker's Algorithm: SAFE | "+order)
  else:self.setstatus("UNSAFE STATE","#b45309","No safe sequence exists for this state.");self.note("Banker's Algorithm: UNSAFE")
  self.showmetrics()
 def detect(self):
  av=self.arr(self.av);al=self.flat(self.al);rq=self.flat(self.req);dead=self.arr([0]*self.p);self.lib.detect_deadlock(self.p,self.r,av,al,rq,dead)
  names=[f"P{i}" for i in range(self.p) if dead[i]]
  if names:self.setstatus("DEADLOCK DETECTED","#b91c1c","Processes: "+", ".join(names));self.note("Detection: "+", ".join(names))
  else:self.setstatus("NO DEADLOCK DETECTED","#16804a","No deadlocked process found in current Request matrix.");self.note("Detection: no deadlock.")
  self.showmetrics()
 def request(self):
  try:
   pi=int(self.pidx.get());v=list(map(int,self.reqv.get().split()))
   if len(v)!=self.r:raise ValueError("Request vector must contain R values.")
   av=self.arr(self.av);al=self.flat(self.al);mx=self.flat(self.mx);rq=self.arr(v);seq=self.arr([0]*self.p)
   result=self.lib.request_resources(self.p,self.r,pi,av,al,mx,rq,seq)
   if result==1:
    self.av=list(av);self.al=[list(al[i*self.r:(i+1)*self.r]) for i in range(self.p)];self.note(f"Request granted to P{pi}; tentative state passed safety check.");self.banker()
   elif result==0:self.note(f"Request denied for P{pi}: insufficient resources or unsafe state.");messagebox.showwarning("Denied","Request cannot be granted safely.")
   else:raise ValueError("Invalid process index, negative request, or request exceeds remaining Need.")
   self.refresh()
  except Exception as e:messagebox.showerror("Request error",str(e))
 def queue_request(self):
  try:
   pi=int(self.pidx.get());v=list(map(int,self.reqv.get().split()))
   if not 0<=pi<self.p or len(v)!=self.r:raise ValueError("Check process index and request-vector length.")
   if any(x<0 for x in v):raise ValueError("Request values cannot be negative.")
   if any(v[j]>self.mx[pi][j]-self.al[pi][j] for j in range(self.r)):raise ValueError("Request exceeds this process's remaining Need.")
   if all(x==0 for x in v):raise ValueError("Enter at least one requested resource.")
   if all(v[j]<=self.av[j] for j in range(self.r)):
    messagebox.showinfo("Resources available","This request can be tested with Submit safe request instead.")
    return
   self.req[pi]=v
   self.note(f"Blocked request recorded for P{pi}: "+" ".join(map(str,v)))
   self.refresh();self.detect()
  except Exception as e:messagebox.showerror("Queued request error",str(e))
 def recover(self):
  try:
   pi=int(self.rec.get())
   if not 0<=pi<self.p:raise ValueError("Process index out of range.")
   av=self.arr(self.av);al=self.flat(self.al);rq=self.flat(self.req);self.lib.release_process_resources(self.p,self.r,pi,av,al,rq)
   self.av=list(av);self.al=[list(al[i*self.r:(i+1)*self.r]) for i in range(self.p)];self.req=[list(rq[i*self.r:(i+1)*self.r]) for i in range(self.p)]
   self.note(f"Recovery: terminated P{pi}; released its allocated resources.");self.refresh();self.banker()
  except Exception as e:messagebox.showerror("Recovery error",str(e))
 def setstatus(self,a,b,c):self.status.set(a);self.status_label.configure(fg=b);self.detail.set(c)
 def showmetrics(self):
  total=sum(self.av)+sum(map(sum,self.al));allocated=sum(map(sum,self.al));s=f"Allocated resource instances: {allocated}\nAvailable resource instances: {sum(self.av)}\nTotal represented: {total}\nUtilization: {allocated/total*100 if total else 0:.1f}%"
  self.metrics.configure(state="normal");self.metrics.delete("1.0","end");self.metrics.insert("end",s);self.metrics.configure(state="disabled")
 def graph(self):
  if not hasattr(self,"canvas"):return
  c=self.canvas;c.delete("all");w=max(c.winfo_width(),500);h=max(c.winfo_height(),350);lx=w*.24;rx=w*.76
  py=[55+i*(h-100)/max(self.p-1,1) for i in range(self.p)];ry=[55+j*(h-100)/max(self.r-1,1) for j in range(self.r)]
  c.create_text(12,12,anchor="nw",text="Solid blue: allocated (R → P)     Dashed red: outstanding request (P → R)",fill="#526174")
  for i,y in enumerate(py):c.create_oval(lx-24,y-17,lx+24,y+17,fill="#dbeafe",outline="#2563eb",width=2);c.create_text(lx,y,text=f"P{i}",fill="#1e3a8a")
  for j,y in enumerate(ry):c.create_rectangle(rx-28,y-18,rx+28,y+18,fill="#dcfce7",outline="#16a34a",width=2);c.create_text(rx,y,text=f"R{j}",fill="#14532d")
  for i in range(self.p):
   for j in range(self.r):
    if self.al[i][j]:c.create_line(rx-28,ry[j],lx+24,py[i],arrow="last",fill="#2563eb",width=2);c.create_text((rx+lx)/2,(ry[j]+py[i])/2-7,text=str(self.al[i][j]),fill="#1d4ed8")
    if self.req[i][j]:c.create_line(lx+24,py[i],rx-28,ry[j],arrow="last",fill="#dc2626",dash=(5,3),width=2)
 def reset(self):
  self.p,self.r=5,3;self.pv.set("5");self.rv.set("3");self.av=[3,3,2];self.al=[[0,1,0],[2,0,0],[3,0,2],[2,1,1],[0,0,2]];self.mx=[[7,5,3],[3,2,2],[9,0,2],[2,2,2],[4,3,3]];self.req=[[0]*3 for _ in range(5)];self.setstatus("READY","#16804a","Sample state restored.");self.refresh();self.note("Sample restored.")
if __name__=="__main__":
 root=tk.Tk();App(root);root.mainloop()
