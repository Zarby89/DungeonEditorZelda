{
	doc->fsize=lastaddr[i];
	break;
}

for(i = 0;i < 31;i++)
((int*) (rom + 0x100004))[i] = lastaddr[i+8];
}
if(move_flag[v]) memmove(rom + r,rom + s,p - s);
doc->rom = rom;
for(i = 0;i < max;i++)
{
		m = l[i];
		q = u[i];
		if(q == 40)continue;
	if(m < base[q] || m > lastaddr[q] || (m > limit[q])) {
		wsprintf(buffer,"Internal error, block %d is outside range %d",i,q);
		MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
	}
}

nochange:for(i = 0;i < 320;i++) {
	blah = (int*)(rom + 0x1794d + i*3); //1794d Overworld overlay?
	*blah &= 0xff000000;
	*blah |= cpuaddr(l[i]);
}
*(int*) (rom+0x882d) = cpuaddr(l[640]) + 0x85000000;
*(int*) (rom+0x8827) = cpuaddr(l[640]) + 0x85000001;
j = cpuaddr(l[668]);
rom[0x9c25] = (unsigned char) (j >> 16);
*(short*) (rom+0x9c2a) = (short) j;
rom[0xcbad] = (unsigned char) (j >> 16);
*(short*)(rom+0xcba2)= (short) j;

for(i = 0;i < 19;i++) {
	blah = (int*) (rom + 0x26cc0 + i*3); //26cc0 dungeon overlay?
	*blah &= 0xff000000;
	*blah |= cpuaddr(l[i + 641]);
}

for(i = 0;i < 8;i++) {
	blah=(int*) (rom + l[640] + i*3);
	*blah &= 0xff000000;
	*blah |= cpuaddr(l[i+660]);
}
for(i = 0;i < 7;i++) {
	if(!(rom[0x1386 + i] & 128)) continue;
	j = cpuaddr(l[i + 669]);
	rom[0x137d + i] = (unsigned char) j;
	rom[0x1386 + i] = (unsigned char) (j >> 8);
	rom[0x138f + i] = (unsigned char) (j >> 16);
}
for(i = 676;i < max;i++) {
	if( l[i] != doc->segs[i-676] ) 
		doc->p_modf = 1;
	doc->segs[i-676] = l[i];
}
if(num < 4096) {
	j = l[num];
	for(i = 0;i<320;i++) {
		if(l[i + 320] == j) t[i] = door_ofs;
	}
}

for(i = 0;i < 320;i++){
	blah = (int*) (rom + 0xf8000 + i*3); //F8000 = room objects pointers
	*blah &= 0xff000000;
	*blah |= cpuaddr(l[i+320]);
	blah = (int*) (rom + 0xf83c0 + i*3); //FF83C0 = room door pointer
	*blah &= 0xff000000;
	*blah |= cpuaddr(l[i + 320] + t[i]);
}
doc->modf = 1;
if(num == 4097)
	return doc->mapexp;
if(num == 4096)
	return doc->tmend3;
if(l[num] >= doc->fsize)
	return 0;

return l[num];
}
//Changesize********************************************
//GetBG3GFX*************************************
unsigned char* GetBG3GFX(char *buf,int size){
	int c,d;
	unsigned char *buf2 = malloc(size);
	size >>= 1;
	for(d=0;d < size;d += 16) {
		for(c = 0;c < 16;c++) 
			buf2[d+d+c]=buf[d+c];
		for(c = 0;c < 16;c++) 
			buf2[d+d+c+16]=0;
		// interesting, it's interlaced with a row of zeroes every
		//other line
	}
	return buf2;
}
//GetBG3GFX**************************************
//GetBlocks#*************************************
void Getblocks(FDOC *doc, int b){
char *rom;
int a;
ZBLOCKS *zbl = doc->blks+b;
char *buf, *buf2;
if(b > 225 || b < 0)
	return;
zbl->count++;
if(zbl->count > 1) 
	return;
rom = (char*) doc->rom;
if(b == 225) {
	buf = Unsnes(rom + 0x80000, 0x7000);
}
 else if(b == 224) {
	buf2 = (char*) GetBG3GFX(rom + 0x70000, 0x2000); //70000 font?
	buf = Unsnes(buf2, 0x2000);
	free(buf2);
}
 else if(b == 223) {
	buf = malloc(0x4000);
	memcpy(buf, rom + 0xc4000, 0x4000);
}
 else {
	a = (rom[0x4f80 + b] << 16) & 0x00FF0000;
	a += (rom[0x505f + b] << 8) & 0x0000FF00;
	a += rom[0x513e + b] & 0x000000FF;
	a = romaddr(a);
	if(b >= 0x73 && b < 0x7f) {
		buf2 = (char*) Make4bpp(rom + a, 0x800);
		buf = Unsnes(buf2, 0x800);
	}
	else {
		buf = (char*) Uncompress(rom + a,0,0);
		if(b >= 220 || (b >= 113 && b < 115)) {
			buf2 = (char*) GetBG3GFX(buf, 0x1000);
			free(buf);
			buf = Unsnes(buf2, 0x1000);
		}
		else {
			buf2 = (char*) Make4bpp(buf, 0x800);
			free(buf);
			buf = Unsnes(buf2, 0x800);
			}
		}
	free(buf2);
	}
zbl->buf = buf;
zbl->modf = 0;
}

//GetBlocks*****************************************
//Releaseblks#***************************************
void Releaseblks(FDOC*doc,int b){
ZBLOCKS *zbl = doc->blks + b;
unsigned char  *buf, *rom, *b2;
int a,c,d,e,f;
if(b > 225 || b < 0)
	return;
zbl->count--;
if(!zbl->count) {
	if(zbl->modf) {
	rom = doc->rom;
		if(b == 225) {
		b2 = Makesnes(zbl->buf, 0xe000); //BG3 data?
		memcpy(rom+0x80000, b2, 0x7000);
		free(b2);
		doc->modf = 1;
		}
	else if(b == 224) {
		b2 = Make2bpp(zbl->buf, 0x2000);
		memcpy(rom + 0x70000, b2, 0x1000);
		free(b2);
		doc->modf = 1;
	}
	 else if(b == 223) {
		memcpy(rom + 0xc4000, zbl->buf, 0x4000);
		doc->modf = 1;
	}
	else {
		a=romaddr((rom[0x4f80 + b] << 16) + (rom[0x505f + b] << 8) + rom[0x513e + b]);
		if(b >= 0x73 && b < 0x7f) {
			buf = Make3bpp(zbl->buf, 0x1000);
			memcpy(rom + a, buf, 0x600);
			doc->modf = 1;
		}
	else {
		if(b >= 220 || (b >= 113 && b < 115)) {
			buf = Make2bpp(zbl->buf, 0x1000);
			b2 = Compress(buf, 0x800, &c, 0);
		}
		 else {
			buf = Make3bpp(zbl->buf, 0x1000);
			b2=Compress(buf, 0x600, &c, 0);
		}
		free(buf);
		f = doc->gfxend;
		for(d = 0;d < 223;d++) {
			e = romaddr( (rom[0x4f80 + d] << 16) + (rom[0x505f + d] << 8) + rom[0x513e + d]);
			if(e < f && e > a) 
				f = e;
		}
		if(doc->gfxend - f + a + c > 0xc4000) {
			free(b2);
			wsprintf(buffer, "Not enough space for blockset %d", b);
			MessageBox(framewnd,buffer, "Bad error happened", MB_OK);
			goto done;
		}
		for(d = 0;d < 223;d++) {
			e = romaddr( (rom[0x4f80 + d] << 16) + (rom[0x505f + d] << 8) + rom[0x513e + d]);
			if(e > a) {
				e = cpuaddr(e + a - f + c);
				rom[0x4f80 + d] = (unsigned char) (e >> 16);
				rom[0x505f + d] = (unsigned char) (e >> 8);
				rom[0x513e + d] = (unsigned char) e;
			}
		}
		memmove(doc->rom + a + c, doc->rom + f, doc->gfxend - f);
		doc->gfxend += a - f + c;
		memcpy(doc->rom + a, b2, c);
		free(b2);
		doc->modf = 1;
		}
	}
}
done:free(zbl->buf);
zbl->buf = 0;
}
}
//Releaseblks*****************************************
//LoadPal#**********************************
void Loadpal(void *ed, unsigned char *rom, int start, int ofs, int len, int pals){
int i,j,k,l;
short* a = (short*) (rom + romaddr(start));
if( ( (DUNGEDIT*) ed)->hpal) {
	PALETTEENTRY pe[16];
	for(i = 0;i < pals;++i) {
		for(j = 0;j < len;++j) {
			l = *(a++);
			pe[j]peRed = (char) ((l & 0x1f) << 3);
			pe[j]peGreen = (char) ((l & 0x3e0) >> 2);
			pe[j]peBlue = (char) ((l & 0x7c00) >> 7);
			pe[j]peFlags = 0;
			}
		SetPaletteEntries(( (DUNGEDIT*) ed)->hpal, (ofs + *(short*) (( (DUNGEDIT*) ed)->pal)) & 255, len, pe);
		ofs += 16;
		}
	}
	else{
		RGBQUAD* pal = ( (DUNGEDIT*) ed)->pal;
		for(i = 0;i < pals;++i) {
			k = ofs;
			for(j = 0;j < len;++j) {
				l = *(a++);
				pal[k]rgbRed = (char) ((l & 0x1f) << 3);
				pal[k]rgbGreen = (char) ((l & 0x3e0) >> 2);
				pal[k]rgbBlue = (char) ((l & 0x7c00) >> 7);
				pal[k]rgbReserved = 0;
				++k;
			}
		ofs += 16;
		}
	}
}
//LoadPal***********************************
void Changeselect(HWND hc,int sel){
	int sc;
	int i;
	RECT rc;
	OVEREDIT*ed;
	ed=(OVEREDIT*)GetWindowLong(hc,GWL_USERDATA);
	GetClientRect(hc,&rc);
	rctop=((ed->sel_select>>2)-ed->sel_scroll)<<5;
	rcbottom=rctop+32;
	ed->selblk=sel;
	if(ed->schflag) {
		for(i=0;i<ed->schnum;i++) if(ed->schbuf[i]==sel) {
			sel=i;
			goto foundblk;
		}
		sel=-1;
	}
foundblk:ed->sel_select=sel;
InvalidateRect(hc,&rc,0);
if(sel==-1) 
	return;

sel>>=2;
sc=ed->sel_scroll;
if(sel>=sc+ed->sel_page) 
	sc=sel+1-ed->sel_page;
else if(sel<sc) 
	sc=sel;

SendMessage(hc,WM_VSCROLL,SB_THUMBPOSITION|(sc<<16),0);
rctop=(sel-ed->sel_scroll)<<5;
rcbottom=rctop+32;
InvalidateRect(hc,&rc,0);
}
const static short nxtmap[4]={
-1,1,-16,16};
const static unsigned char bg2_ofs[]={
0,32,64,96,128,160,192,224,1};
//Initroom********************************
void Initroom(DUNGEDIT *ed, HWND win){
	unsigned char *buf2;
	int i,j,l,m;
	unsigned char *rom = ed->ewdoc->rom;
	//*****Zarby Comment : No idea what is suposed to be commented or not*****
	// equate the romfile here with ed's
	buf2 = rom + (ed->hbuf[1] << 2) + 0x75460; //75460-$75503 - used in dungeons to set $0AB6, $0AAC, $0AAD, and $0AAE
	//Loadpal(ed,rom,0x1bd734+*((unsigned short*)(rom+0xdec4b+buf2[0])),0x21,15,6);
	//I didn't comment the above out, to the best of my knowledge, -MON
	 j = 0x6073 + (ed->gfxtmp << 3);
	 ed->gfxnum = ed->hbuf[2];
	 ed->sprgfx = ed->hbuf[3];
	 ed->palnum = ed->hbuf[1];
	 l = 0x5d97 + (ed->hbuf[2] << 2);
	 // gives an offset (bunched in 4's)
	 // determine blocsets 0-7
	for(i = 0;i < 8;i++)
		ed->blocksets[i] = rom[j++];
	// these are uniquely determined
    ed->blocksets[8] = (rom + 0x1011e) [ed->gfxnum];
	ed->blocksets[9] = 0x5c;
	ed->blocksets[10] = 0x7d;
// rewrite blocksets 3-6, if necessary
	for(i = 3;i < 7;i++){
		m = rom[l++];
		if(m)ed->blocksets[i] = m;
	}
	l = 0x5c57 + (ed->sprgfx << 2);
	// determine blocksets 11-14, which covers them all
	for(i = 0;i < 4;i++) 
		ed->blocksets[i + 11] = rom[l + i] + 0x73;
		//get the block graphics for our tilesets?
	for(i = 0;i < 15;i++)
		Getblocks(ed->ewdoc, ed->blocksets[i]);
	
	ed->layering = (unsigned char) (ed->hbuf[0] & 0xe1);
	// take bits 2-4, shift right 2 to get a 3 bit number
	ed->coll = (unsigned char) ((ed->hbuf[0] & 0x1c) >> 2);
	ed->modf = ed->hmodf = 0;
	 // the header is unmodified, nor is the room, so far
	 // if something changes, the flag will be set
	 SetDlgItemInt(win, 3019, ed->buf[0] & 15, 0);
	 // floor 1SetDlgItemInt(win, 3021, ed->buf[0] >> 4, 0);
	 // floor 2SetDlgItemInt(win, 3041, ed->gfxnum, 0);
	 //blocksetSetDlgItemInt(win, 3043, ed->palnum, 0);
	//palette
 }
 
//Initroom******************************//
LoadHeadervoid LoadHeader(DUNGEDIT *ed,int map){
	// we are passed in a dungeon editing window, and a map number (room number)
	int i, j, l, m;
	// lower limit for the header offset 
	// counter variable for looping through all dungeon rooms
	// size of the header
	// upper limit for he header offset
	int headerAddrLocal = ed->ewdoc->headerLocation;
	unsigned char *rom = ed->ewdoc->rom;
	i = ( (short*) ( rom + headerAddrLocal) )[map];
	l = 14;
	// sort through all the other header offsets
	for(j = 0;j < 0x140;j++){
	// m gives the upper limit for the header
	// if is less than 14 bytes from i
	m = ( (short*) (rom + headerAddrLocal))[j];
		if( (m > i) && (m < i + 14) ){
			l = m - i;
			break;
		}
	}
	ed->hsize = l;
 // determine the size of the header
 // FIX!!!!
 memcpy(ed->hbuf, rom + 0x28000 + i, 14);
 // copy 14 bytes from the i offset
 }
//LoadHeader********************************
//Openroom********************************
void Openroom(DUNGEDIT *ed, int map){
	int i,j,l;
	unsigned char *buf;
	unsigned short k;
	unsigned char *rom;
	HWND win = ed->dlg;
	ed->mapnum = map;
	 // the room number, ranging from 0 to 295
	ed->ewdoc->dungs[map] = win;
	rom = ed->ewdoc->rom;
	// get the base address for this room
	buf = rom + romaddr( *(int*) (rom + 0xf8000 + map * 3));
	i = 2;
	 //we'll step in by 2 bytes here
	ed->selobj=0;
	ed->selchk=0;
	for(j = 0;j < 6;j++) {
		ed->chkofs[j] = i;
		for(;;) {
			k = *(unsigned short*)(buf + i);
			if(k == 0xffff) // code indicating to stopbreak;
			if(k == 0xfff0) // code indicating to do this loop{
			j++;
			ed->chkofs[j] = i + 2;
			for(;;) {
				i += 2;
				k = *(unsigned short*)(buf+i);
				if(k == 0xffff) goto end;
				if(!ed->selobj)ed->selobj = i,ed->selchk = j;
			}
		}
		else 
			i += 3;
	if(!ed->selobj) // if there is no selected object, pick one{
		ed->selobj = i - 3,ed->selchk = j;
		}
	}
	j++;
	ed->chkofs[j] = i+2;
	end:i += 2;
	}
	// now we know where the room data separates into special subsections
	j = ed->selchk & -2;
	 // AND by 0xFFFFFFFE Makes j into an even number
	CheckDlgButton(win, 3034, j ? BST_UNCHECKED : BST_CHECKED);
	CheckDlgButton(win, 3035, (j == 2) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(win, 3036, (j == 4) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(win, 3046, (j == 6) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(win, 3057, (j == 7) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(win, 3058, (j == 8) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(win, 3059, (j == 9) ? BST_CHECKED : BST_UNCHECKED);
	// do some initial settings, determine which radio button to check first 1, 2, 3,
	ed->len = i;
	 // size of the buffer
	 ed->buf = malloc(i);
	 // generate the buffer of size 
	 imemcpy(ed->buf, buf, i);
	// copy the data from buf to 
	ed->bufSetDlgItemInt(win,3029, buf[1] >> 2, 0);
	 // this is the "layout", ranging from 0-7
	 LoadHeader(ed,map);
	 // load the header information
	 Initroom(ed,win);
	 // wsprintf(buffer,"Room %d",map);
	 // prints the room string in the upper left hander corner
	 SetDlgItemText(win, 3000, buffer);
	 // ditto
	 if(map > 0xff){
	// If we're in the upper range, certain buttons might be grayed out so we can't
	// get back to the lower range
		for(i = 0;i < 4;i++) {
			EnableWindow(GetDlgItem(win, 3014 + i),((map + nxtmap[i]) & 0xff) < 0x28);
		}
	}
	else {
		EnableWindow(GetDlgItem(win,3014),1);
		EnableWindow(GetDlgItem(win,3015),1);
		EnableWindow(GetDlgItem(win,3016),1);
		EnableWindow(GetDlgItem(win,3017),1);
	}
	CheckDlgButton(win, 3034, BST_CHECKED);
	CheckDlgButton(win, 3035, BST_UNCHECKED);
	CheckDlgButton(win, 3036, BST_UNCHECKED);
	for(i = 0;i < 9;i++){
		if(ed->layering == bg2_ofs[i]) {
			SendDlgItemMessage(win, 3039, CB_SETCURSEL, i, 0);
			break;
		}
	}
	SendDlgItemMessage(win, 3045, CB_SETCURSEL, ed->coll, 0);
	SetDlgItemInt(win, 3050, ed->sprgfx, 0);
	buf = rom + 0x50000 + ( (short*) (rom + ed->ewdoc->dungspr) )[map];
	for(i = 1;;i += 3) {
		if(buf[i] == 0xff)break;
	}
	ed->esize = i;
	ed->ebuf = malloc(i);
	memcpy(ed->ebuf,buf,i);
	CheckDlgButton(win, 3060, *ed->ebuf ? BST_CHECKED:BST_UNCHECKED);
	buf = rom + 0x10000 + ((short*) (rom + 0xdb69))[map]; //db69 Secrets data
	for(i = 0;;i += 3)
		if(*(short*)(buf + i) == -1) 
			break;
		
	ed->ssize = i;
	ed->sbuf = malloc(i);
	memcpy(ed->sbuf,buf,i);
	buf = rom + 0x2736a;
	l = *(short*)(rom + 0x88c1);
	for(i = 0;;i += 2){
		if(i >= l){
			ed->tsize = 0;
			ed->tbuf = 0;
			break;
		}
		j = i;
		for( ;;) {
		i += 2;
		if(*(short*)(buf + i)==-1)break;
		}
		if(*(short*)(buf + j) == map){
			memcpy(ed->tbuf = malloc(ed->tsize = i - j),buf + j,i-j);
			break;
		}
	}
}
//OpenRoom*****************************
//SaveDungSecrets**********************
void Savedungsecret(FDOC*doc,int num,unsigned char*buf,int size){
int i,j,k;
int adr[0x140];
unsigned char*rom=doc->rom;
for(i=0;i<0x140;i++)
	adr[i]=0x10000+((short*)(rom+0xdb69))[i];
	j=adr[num];
	k=adr[num+1];
	if(*(short*)(rom+j)==-1) {
		if(!size) 
			return;
		j+=size+2;
		adr[num]+=2;
	}
	else {
		if(!size) {
			if(j>0xdde9) {
				j-=2;
				adr[num]-=2;
				}
			}
			else 
				j+=size;
		}
		if(*(short*)(rom+k)!=-1) 
			k-=2;
		if(adr[0x13f]-k+j>0xe6b0) {
			MessageBox(framewnd,"Not enough space for secret items","Bad error happened",MB_OK);
			return;
		}
	memmove(rom+j,rom+k,adr[0x13f]+2-k);
	if(size) 
		memcpy(rom+adr[num],buf,size);
	if(j==k) 
		return;
	
	((short*)(rom+0xdb69))[num]=adr[num];
	for(i=num+1;i<0x140;i++) {
		((short*)(rom+0xdb69))[i]=adr[i]+j-k;
	}
}

Savesprites(FDOC*doc,int num,unsigned char*buf,int size){
	int i,k,l,m,n;
	int adr[0x288];
	unsigned char*rom=doc->rom;
	if(size) size++;
	for(i=0;i<0x160;i++)
		adr[i]=((short*)(rom+0x4c881))[i]+0x50000;
	k=doc->dungspr-0x2c0;
	for(;i<0x288;i++)
		adr[i]=((short*)(rom+k))[i]+0x50000;
	if(num&65536) {num&=65535;l=adr[num];
	for(i=0;i<0x288;i++)
		if(adr[i]==l) {
			adr[num]=0x4cb41;
			goto nochg;
		}
	size=0;
	}
	else l=adr[num];
	if(l==0x4cb41 || l==doc->sprend-2) 
		m=l=((num>=0x160)?(doc->dungspr+0x300):0x4cb42);
	else {
		m=(num>=0x160)?doc->sprend:doc->dungspr;
		for(i=0;i<0x288;i++) {
			n=adr[i];
			if(n<m && n>l) 
				m=n;
		}
	}
	if(size==2 && !buf[0]) size=0;
		if(doc->sprend+l-m+size>0x4ec9f) 
		{
			MessageBox(framewnd,"Not enough space for sprites","Bad error happened",MB_OK);
			return 1;
		}
	if(size+l!=m) {
		memmove(rom+l+size,rom+m,doc->sprend-m);
		doc->sprend+=size-m+l;
		for(i=0;i<0x288;i++)
			if(adr[i]>=m) 
				adr[i]+=size-m+l;
	}
	if(!size) 
		adr[num]=((num>=0x160)?(doc->sprend-2):0x4cb41);
	else {
		memcpy(rom+l,buf,size-1);
		rom[l+size-1]=0xff;
		adr[num]=l;
	}
	if(doc->dungspr>=l)doc->dungspr+=size-m+l;
	nochg:for(i=0;i<0x160;i++)
		((short*)(rom+0x4c881))[i]=adr[i];
	k=doc->dungspr-0x2c0;
	for(;i<0x288;i++)
		((short*)(rom+k))[i]=adr[i];
	
	return 0;
}

//Saveroom******************************************
void Saveroom(DUNGEDIT *ed){
	unsigned char *rom = ed->ewdoc->rom;
	// get our romfile from the Dungedit struct
	int headerAddrLocal = ed->ewdoc->headerLocation;
	int i,j,k,l,m,n;
	if( !ed->modf ) // if the dungeon map hasn't been modified, do nothing
		return;
// if it has save it
	if( ed->ewparam >= 0x8c) // if the map is an overlay 
	{
		i = Changesize(ed->ewdoc, ed->ewparam + 0x301f5, ed->len-2);
		if(!i) return;
		memcpy(rom + i, ed->buf + 2, ed->len - 2);
	}
	else{
		Savesprites(ed->ewdoc, 0x160 + ed->mapnum, ed->ebuf, ed->esize);
		for(i = 5;i >= 1;i -= 2) {
			if(ed->chkofs[i] != ed->chkofs[i+1]) 
				break;
		}
		door_ofs = ed->chkofs[i];
		i = Changesize(ed->ewdoc, ed->mapnum + 0x30140, ed->len);
		if( !i )
			return;
		
		memcpy(rom + i, ed->buf, ed->len);
		Savedungsecret(ed->ewdoc, ed->mapnum, ed->sbuf, ed->ssize);
		k = *(short*)(rom + 0x88c1);
		if(*(short*)(rom + 0x2736a) == -1)k = 0;
		for(i = 0;i < k;i += 2){
			j = i;
			for(;;){
				j+=2;
				if(*(short*)(rom+0x2736a+j) == -1)
					break;
			}
		if(*(short*)(rom+0x2736a+i) == ed->mapnum){
			if(!ed->tsize) 
				j += 2;
			if(k+i+ed->tsize-j>0x120)
				goto noroom;
			
			memmove(rom+0x2736a+i+ed->tsize,rom+0x2736a+j,k-j);
			memcpy(rom+0x2736a+i,ed->tbuf,ed->tsize);
			k += i + ed->tsize - j;
			break;
		}
		else 
			i = j;
	}
	
	if(ed->tsize && i == k){
		j = ed->tsize + 2;
		if(k + j > 0x120) 
			noroom: MessageBox(framewnd,"Not enough room for torches","Bad error happened",MB_OK);
		else {
			memcpy(rom + 0x2736a + k, ed->tbuf, ed->tsize);
			*(short*)(rom + 0x2736a + k + j - 2) = -1;
			k += j;
		}
	}
	if(!k) 
		k = 4, *(int*)(rom + 0x2736a) == -1;
	*(short*)(rom + 0x88c1) = k;
	if(m = ed->hmodf)// if the headers have been modified, save them
	{
		i = 0x28000 + ((short*)(rom + headerAddrLocal))[ed->mapnum];
		// some sort of lower bound
		k = 0x28000 + *((short*)(rom + 0x27780));
		// some sort of upper bound
		for(j = 0;j < 0x140;j++){
			if(j == ed->mapnum) // if we hit the map number we're currently on, keep moving
				continue;
			if(0x28000 + ((short*)(rom + headerAddrLocal))[j] == i){
				if(m > 1)
					goto headerok;
				wsprintf(buffer,"The room header of room %d is reused Modify this one only?",ed->mapnum);
				if(MessageBox(framewnd,buffer,"Bad error happened",MB_YESNO)==IDYES){
					headerok:k = i;
					goto changeroom;
				}
			break;
			}
		}
		
		for(j = 0;j < 0x140;j++) {
			l = 0x28000 + ((short*)(rom + headerAddrLocal))[j];
			if(l > i && l < k)
				k = l;
		}
		changeroom:
		if(m > 1){
			((short*)(rom + headerAddrLocal))[ed->mapnum] = ((short*)(rom + headerAddrLocal))[m-2];
			n = 0;
		}
		else 
			n = ed->hsize;
		if(*((short*)(rom + 0x27780)) + i + n - k > 0)
			MessageBox(framewnd,"Not enough room for room header","Bad error happened",MB_OK);
		else {
			memmove(rom + i + n,rom + k,0x28000 + *((short*)(rom + 0x27780)) - k);
			*((short*)(rom + 0x27780)) += i + n - k;
			memcpy(rom + i,ed->hbuf,n);
			for(j = 0;j < 0x140;j++)
				if(j != ed->mapnum && ((short*)(rom + headerAddrLocal))[j] + 0x28000 >= k)
					((short*)(rom + headerAddrLocal))[j] += i + n - k;
		}
		if(n) {
			rom[i] = ed->layering|(ed->coll<<2);
			rom[i+1] = ed->palnum;
			rom[i+2] = ed->gfxnum;
			rom[i+3] = ed->sprgfx;
		}

	}
}
ed->modf=0;
ed->ewdoc->modf=1;
}
//SaveRoom***********************************
//Closeroom*********************************
int Closeroom(DUNGEDIT *ed){
	int i;
	if(ed->ewdoc->modf != 2 && ed->modf) {
		if(ed->ewparam < 0x8c)
			wsprintf(buffer,"Confirm modification of room %d?",ed->mapnum);
		else wsprintf(buffer,"Confirm modification of overlay map?");
		switch(MessageBox(framewnd,buffer,"Dungeon editor",MB_YESNOCANCEL)) {
			case IDYES:Saveroom(ed);
			break;
			case IDCANCEL:return 1;
		}
	}
	for(i = 0;i < 15;i++) Releaseblks(ed->ewdoc,ed->blocksets[i]);
		if(ed->ewparam<0x8c) 
			ed->ewdoc->dungs[ed->mapnum] = 0;
	return 0;
}
//Closeroom**********************************
void fill4x2(unsigned char*rom,short*nbuf,short*buf){
int i,j,k,l,m;
	for(l=0;l<4;l++) {
		i=((short*)(rom+0x1b02))[l]>>1;
		for(m=0;m<8;m++) {
			for(k=0;k<8;k++) {
				for(j=0;j<j++) {
					nbuf[i]=buf[0];
					nbuf[i+1]=buf[1];
					nbuf[i+2]=buf[2];
					nbuf[i+3]=buf[3];
					nbuf[i+64]=buf[4];
					nbuf[i+65]=buf[5];
					nbuf[i+66]=buf[6];
					nbuf[i+67]=buf[7];
					i+=128;
				}
				i-=252;
			}
			i+=224;
		}
	}
}

void len1(){
	if(!dm_l) 
		dm_l=0x20;
}
void len2(){
	if(!dm_l) 
		dm_l=0x1a;
}
void draw2x2(){
	dm_wr[0]=dm_rd[0];
	dm_wr[64]=dm_rd[1];
	dm_wr[1]=dm_rd[2];
	dm_wr[65]=dm_rd[3];
	dm_wr+=2;
}
void drawXx4(int x){
	while(x--) {
		dm_wr[0]=dm_rd[0];
		dm_wr[64]=dm_rd[1];
		dm_wr[128]=dm_rd[2];
		dm_wr[192]=dm_rd[3];
		dm_rd+=4;
		dm_wr++;
	}
}
void drawXx4bp(int x){
	while(x--) {
		dm_buf[0x1000+dm_x]=dm_buf[dm_x]=dm_rd[0];
		dm_buf[0x1040+dm_x]=dm_buf[0x40+dm_x]=dm_rd[1];
		dm_buf[0x1080+dm_x]=dm_buf[0x80+dm_x]=dm_rd[2];
		dm_buf[0x10c0+dm_x]=dm_buf[0xc0+dm_x]=dm_rd[3];
		dm_x++;
		dm_rd+=4;
	}
}
void drawXx3bp(int x){
	while(x--) {
		dm_buf[0x1000+dm_x]=dm_buf[dm_x]=dm_rd[0];
		dm_buf[0x1040+dm_x]=dm_buf[0x40+dm_x]=dm_rd[1];
		dm_buf[0x1080+dm_x]=dm_buf[0x80+dm_x]=dm_rd[2];
		dm_x++;
		dm_rd+=3;
	}
}
void drawXx3(int x){
	while(x--) {
		dm_wr[0]=dm_rd[0];
		dm_wr[64]=dm_rd[1];
		dm_wr[128]=dm_rd[2];
		dm_rd+=3;
		dm_wr++;
	}
}
void draw3x2(){
	int x=2;
	while(x--) {
		dm_wr[0]=dm_rd[0];
		dm_wr[1]=dm_rd[1];
		dm_wr[2]=dm_rd[2];
		dm_wr[64]=dm_rd[3];
		dm_wr[65]=dm_rd[4];
		dm_wr[66]=dm_rd[5];
	}
}
void draw1x5(){
	dm_wr[0]=dm_rd[0];
	dm_wr[64]=dm_rd[1];
	dm_wr[128]=dm_rd[2];
	dm_wr[192]=dm_rd[3];
	dm_wr[256]=dm_rd[4];
}
void draw8fec(short n){
	dm_wr[0]=dm_rd[0];
	dm_wr[1]=dm_rd[1];
	dm_wr[65]=dm_wr[64]=n;
}
void draw9030(short n){
	dm_wr[1]=dm_wr[0]=n;
	dm_wr[64]=dm_rd[0];
	dm_wr[65]=dm_rd[1];
}
void draw9078(short n){
	dm_wr[0]=dm_rd[0];
	dm_wr[64]=dm_rd[1];
	dm_wr[65]=dm_wr[1]=n;
}
void draw90c2(short n){
	dm_wr[64]=dm_wr[0]=n;
	dm_wr[1]=dm_rd[0];
	dm_wr[65]=dm_rd[1];
}
void draw4x2(int x){
	while(dm_l--) {
		dm_wr[0]=dm_rd[0];
		dm_wr[1]=dm_rd[1];
		dm_wr[2]=dm_rd[2];
		dm_wr[3]=dm_rd[3];
		dm_wr[64]=dm_rd[4];
		dm_wr[65]=dm_rd[5];
		dm_wr[66]=dm_rd[6];
		dm_wr[67]=dm_rd[7];
		dm_wr+=x;
	}
}
void draw2x6(short*nbuf){
	int m;
	dm_wr=nbuf+dm_x;
	m=6;
	while(m--) {
		dm_wr[0]=dm_rd[0];
		dm_wr[1]=dm_rd[6];
		dm_wr+=64;
		dm_rd++;
	}
}
void drawhole(int l,short*nbuf){
	draw2x6(nbuf);
	dm_x+=2;
	dm_rd+=6;
	while(l--) {
		dm_buf[dm_x]=dm_buf[dm_x+64]=dm_buf[dm_x+128]=dm_buf[dm_x+192]=dm_buf[dm_x+256]=dm_buf[dm_x+320]=dm_rd[0];
		dm_x++;
	}
	draw2x6(nbuf);
}
void draw4x4X(int n){
	int x;
	while(n--) {
		x=2;
		while(x--) {
			dm_wr[0]=dm_rd[0];
			dm_wr[1]=dm_rd[1];
			dm_wr[2]=dm_rd[2];
			dm_wr[3]=dm_rd[3];
			dm_wr[64]=dm_rd[4];
			dm_wr[65]=dm_rd[5];
			dm_wr[66]=dm_rd[6];
			dm_wr[67]=dm_rd[7];
			dm_wr+=128;
		}
		dm_wr-=252;
	}
}
void draw12x12(){
	int m,l;
	l=12;
	while(l--) {
		m=12;
		while(m--) 
			dm_buf[dm_x+0x1000]=dm_rd[0],dm_x++,dm_rd++;
		dm_x+=52;
	}
}
void draw975c(){
	unsigned char m;
	m=dm_l;
	dm_wr[0]=dm_rd[0];
	while(m--) 
		dm_wr[1]=dm_rd[3],dm_wr++;
	dm_wr[1]=dm_rd[6];
	dm_wr[2]=dm_wr[3]=dm_wr[4]=dm_wr[5]=dm_rd[9];
	m=dm_l;
	dm_wr[6]=dm_rd[12];
	while(m--) 
		dm_wr[7]=dm_rd[15],dm_wr++;
	dm_wr[7]=dm_rd[18];
	dm_tmp+=64;
	dm_wr=dm_tmp;
}
void draw93ff(){
	int m;
	short*tmp;
	m=dm_l;
	tmp=dm_wr;
	dm_wr[0]=dm_rd[0];
	while(m--) 
		dm_wr[1]=dm_rd[1],dm_wr[2]=dm_rd[2],dm_wr+=2;
	dm_wr[1]=dm_rd[3];
	tmp+=64;
	dm_wr=tmp;
}

void drawtr(){
	unsigned char n=dm_l,l;
	for(l=0;l<n;l++) 
		dm_wr[l]=*dm_rd;
}

void tofront4x7(int n){
	int m=7;
	while(m--) {
		dm_buf[n]|=0x2000;
		dm_buf[n+1]|=0x2000;
		dm_buf[n+2]|=0x2000;
		dm_buf[n+3]|=0x2000;
		n+=64;
	}
}

void tofront5x4(int n){
	int m=5;
	while(m--) {
		dm_buf[n]|=0x2000;
		dm_buf[n+64]|=0x2000;
		dm_buf[n+128]|=0x2000;
		dm_buf[n+192]|=0x2000;
		n++;
	}
}

void tofrontv(int n){
	int m=n;
	n&=0x783f;
	while(n!=m) {
		dm_buf[n]|=0x2000;
		dm_buf[n+1]|=0x2000;
		dm_buf[n+2]|=0x2000;
		dm_buf[n+3]|=0x2000;
		n+=64;
	}
}
void tofronth(int n){
	int m=n;
	n&=0x7fe0;
	while(n!=m) {
		dm_buf[n]|=0x2000;
		dm_buf[n+64]|=0x2000;
		dm_buf[n+128]|=0x2000;
		dm_buf[n+192]|=0x2000;
		n++;
	}
}
void drawadd4(unsigned char*rom,int x){
	int m=4;
	dm_rd=(short*)(rom+0x1b52+((unsigned short*)(rom+0x4e06))[dm_l]);
	while(m--) {
		dm_buf[x+0x1040]=dm_rd[0];
		dm_buf[x+0x1080]=dm_rd[1];
		dm_buf[x+0xc0]=dm_rd[2];
		dm_rd+=3;
		x++;
	}
	x-=4;
	tofrontv(x+0x100);
}
void drawaef0(unsigned char*rom,int x){
	int m=2;
	dm_rd=(short*)(rom+0x1b52+((unsigned short*)(rom+0x4ec6))[dm_l]);
	while(m--) {
		dm_buf[x+0x1001]=dm_rd[0];
		dm_buf[x+0x1041]=dm_rd[1];
		dm_buf[x+0x1081]=dm_rd[2];
		dm_buf[x+0x10c1]=dm_rd[3];
		dm_rd+=4;
		x++;
	}
	dm_buf[x+1]=dm_rd[0];
	dm_buf[x+65]=dm_rd[1];
	dm_buf[x+129]=dm_rd[2];
	dm_buf[x+193]=dm_rd[3];
	x+=2;
	while(x&0x3f) {
		dm_buf[x]|=0x2000;
		dm_buf[x+64]|=0x2000;
		dm_buf[x+128]|=0x2000;
		dm_buf[x+192]|=0x2000;
		x++;
	}
}
const static char obj_w[64]={
4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,4,4,4,4,2,2,2,2,4,2,2,2,2,2,4,4,4,4,2,2,4,4,4,2,6,4,4,4,4,4,4,4,2,4,4,10,4,4,4,4,24,3,6,1};
const static char obj_h[64]={
4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,2,2,2,2,4,3,2,2,2,3,5,3,4,4,3,2,5,4,2,3,3,4,4,4,4,4,4,4,2,2,2,4,3,3,3,3,6,3,3,7};
const static char obj2_w[128]={
4,4,4,1,1,1,1,1,1,1,1,1,1,16,1,1,2,2,5,2,4,10,2,16,2,2,2,4,4,4,4,4,4,4,2,2,2,2,4,4,4,4,44,2,4,14,28,2,2,4,4,4,3,3,6,6,3,3,4,4,6,6,2,2,2,2,2,2,2,2,2,4,4,2,2,8,6,6,4,2,2,2,2,2,14,3,2,2,2,2,4,3,6,6,2,2,3,3,22,2,2,2,4,4,4,3,3,4,4,4,3,3,4,8,10,64,8,2,8,8,8,4,4,20,2,2,2,1};
const static char obj2_h[128]={
3,5,7,1,1,1,1,1,1,1,1,1,1,4,1,1,2,2,8,2,3,8,2,4,2,2,2,4,4,4,4,4,4,4,2,2,2,2,4,4,4,4,44,2,4,14,25,2,2,3,3,4,2,2,3,3,2,2,6,6,4,4,2,2,2,2,2,2,2,2,2,4,4,2,2,3,8,3,3,2,2,2,2,2,14,5,2,2,2,2,2,5,4,3,2,2,6,6,13,2,2,2,4,3,3,4,4,4,3,3,4,4,10,8,8,64,8,2,3,3,8,3,4,8,2,2,2,1};
const static char obj3_w[248]={
0,0,0,0,0,-4,-4,0,0,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,2,3,2,2,2,2,2,2,2,2,2,2,2,2,13,13,1,1,0,3,1,-2,-2,-2,-4,-4,-4,-2,-4,-12,2,2,2,2,2,2,2,2,2,2,0,0,-12,2,2,2,2,1,2,2,0,1,-8,-8,1,1,1,1,2,2,5,-2,22,2,4,4,4,4,4,4,2,2,1,1,1,2,2,1,1,4,1,1,4,4,2,3,3,2,1,1,2,1,2,1,2,2,3,3,3,3,3,3,2,2,2,1,1,1,1,1,2,4,4,2,2,4,2,2,1,1,1,1,1,1,1,1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,1,7,7,0,2,2,0,0,0,0,0,0,-2,0,0,1,1,0,12,0,0,0,0,0,0,0,0,0,1,1,3,3,1,1,0,0,1,1,1,1,0,4,0,4,0,8,2,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
const static char obj3_h[248]={
2,4,4,4,4,4,4,2,2,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,3,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,1,1,4,1,1,4,4,3,4,3,3,8,4,2,1,1,1,1,1,1,1,1,5,3,2,2,2,3,4,4,4,1,3,3,2,1,2,2,1,1,1,1,3,3,3,2,1,0,0,0,0,0,-4,-4,0,0,3,0,0,13,13,1,1,0,3,1,-2,-2,-2,-4,-4,-12,0,0,-12,1,0,1,-8,-8,-4,-4,-4,-4,2,2,-2,5,-2,22,7,7,0,0,3,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,1,1,1,4,1,1,4,4,4,2,2,4,2,2,2,1,1,0,6,0,0,0,0,0,0,0,0,0,1,1,0,0,1,1,0,0,1,1,1,1,0,4,0,4,0,5,2,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};
const static char obj3_m[248]={
2,2,2,2,2,6,6,2,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,4,1,0,6,6,4,6,8,8,4,6,14,1,1,1,1,1,1,1,1,2,2,4,4,14,2,2,2,2,1,2,2,2,0,12,12,0,0,0,0,2,2,1,4,1,2,2,2,2,2,6,6,2,2,1,1,1,1,1,0,0,4,1,0,6,6,6,8,8,14,1,1,14,1,2,0,12,12,8,8,8,8,2,2,6,1,4,1,1,1,1,1,2,2,2,2,2,4,2,2,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1,4,1,1,2,2,2,2,2,4,4,2,2,0,0,4,2,4,3,4,4,4,4,4,4,4,0,0,1,1,0,0,4,4,0,0,0,0,3,4,4,4,4,2,2,2,4,4,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
const static unsigned char obj3_t[248]={
0,2,2,1,1,1,1,1,1,97,65,65,97,97,65,65,97,97,65,65,97,97,65,65,97,97,65,65,97,97,65,65,97,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,128,130,130,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,130,130,128,128,129,129,129,129,129,129,129,129,129,129,129,129,65,65,65,113,65,65,65,65,113,65,65,65,113,129,129,129,1,1,1,1,1,1,2,2,2,2,1,1,1,1,1,1,3,3,3,3,3,3,3,3,3,3,3,1,1,20,4,1,1,3,3,1,1,1,1,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

void getobj(unsigned char*map){
	unsigned short i;
	unsigned char j;
	i=*(unsigned short*)map;
	j=i&0xfc;
	if(j==0xfc) {
		dm_x=((i&3)<<4)|(i>>12);
		map++;
		i=*(unsigned short*)map;
		dm_x+=((i&15)<<8)|((i&0xc000u)>>8);
		dm_k=((i&0x3f00)>>8)+0x100;
	}
	else {
		dm_x=((j>>2)|((i&0xfc00u)>>4));
		dm_k=map[2];
		if(dm_k<0xf8)
			dm_l=((i&3)<<2)|((i&0x300u)>>8);
	else 
		dm_l=(i&3)|((i&0x300u)>>6);
	}
}
char sprname[0x11c][16];
HDC objdc;
HBITMAP objbmp;
void Getstringobjsize(char*str,RECT*rc){
	GetTextExtentPoint(objdc,str,strlen(str),(LPSIZE)&(rc->right));
	rc->bottom++;
	rc->right++;
	if(rc->bottom<16) 
		rc->bottom=16;
	if(rc->right<16) 
		rc->right=16;
	rc->right+=rc->left;
	rc->bottom+=rc->top;
}
const static char obj4_h[4]={5,7,11,15};
const static char obj4_w[4]={8,16,24,32};

void Getdungobjsize(int chk,RECT*rc,int n,int o,int p){
	int a,b,c,d,e;
	char*f;
	switch(chk) {
		case 0: 
		case 2: 
		case 4:
		if(dm_k>=0x100)
			rc->right=obj_w[dm_k-0x100]<<3,rc->bottom=obj_h[dm_k-0x100]<<3;
		else if(dm_k>=0xf8)
			rc->right=obj2_w[(dm_k-0xf8<<4)+dm_l]<<3,rc->bottom=obj2_h[(dm_k-0xf8<<4)+dm_l]<<3;
		else {
			c=0;
			d=dm_l;
			e=obj3_t[dm_k];
			switch(e&15) {
				case 0:
				len1();
				break;
				case 1:
				dm_l++;
				break;
				case 2:
				len2();
				break;
				case 3:
				c=((dm_l&3)+1)*obj3_m[dm_k];
				dm_l>>=2;
				dm_l++;
				break;
				case 4:
				c=obj4_h[dm_l>>2]+3<<1;
				dm_l=obj4_w[dm_l&3];
				if(e&16) 
					rc->left-=dm_l<<3;
				break;
			}
			dm_l*=obj3_m[dm_k];
			switch(e&192) {
				case 0:
				a=dm_l;
				b=c;
				break;
				case 64:
				a=dm_l;
				b=dm_l;
				break;
				case 128:
				a=0;
				b=dm_l;
				break;
			}
			if(e&32) 
				rc->top-=b+((e&16)?2:4)<<3;
			
			a+=obj3_w[dm_k];
			b+=obj3_h[dm_k];
			rc->right=a<<3;
			rc->bottom=b<<3;
			dm_l=d;
		}
		if(dm_k==0xff) {
			if(dm_l==8) 
				rc->left-=16;
			else if(dm_l==3) 
				rc->left=-n,rc->top=-o;
		}
		else if(dm_k==0xfa) {
			if(dm_l==14) 
				rc->left+=16,rc->top+=32;
		else if(dm_l==10) 
			rc->left=80-n,rc->top=64-o;
		}
		break;
		case 1: 
		case 3: 
		case 5:
		switch(dm_k) {
			case 0:
			if(dm_l==24) {
				rc->right=176;
				rc->bottom=96;
			}
			else if(dm_l==25) 
				rc->right=rc->bottom=32;
			else if(dm_dl>8) {
				rc->top-=120;
				rc->bottom=144;
				rc->right=32;
			}
			else if(dm_dl>5) {
				rc->top-=72;
				rc->bottom=96;
				rc->right=32;
			}
			else 
				rc->right=32,rc->bottom=24;
				break;
			case 1:
			if(dm_l==5 || dm_l==6) {
				rc->right=96;
				rc->bottom=64;
				rc->left-=32;
				rc->top-=32;
			}
			else if(dm_l==2 || dm_l==7 || dm_l==8) {
				rc->right=rc->bottom=32;
			}
			else 
				 rc->right=32,rc->bottom=24,rc->top+=8;
			break;
			case 2:
			rc->bottom=32;
			if(dm_dl>8) {
				rc->left-=104;
				rc->right=128;
			}
			else if(dm_dl>5) {
				rc->left-=56;
				rc->right=80;
			}
			else rc->right=24;
				break;
			case 3:
			rc->right=24,rc->bottom=32,rc->left+=8;
		}
		break;
		case 8: 
		case 9:
		rc->right=16;
		rc->bottom=16;
		break;
		case 7:
		Getstringobjsize(cur_sec,rc);
		goto blah2;
		case 6:if(dm_k>=0x11c) f="Crash";
		else 
			f=sprname[dm_k];
		Getstringobjsize(f,rc);
		blah2:
		if(rc->right>512-n) 
			rc->right=512-n;
		if(rc->bottom>512-o) 
			rc->bottom=512-o;
		goto blah;
	}

	rc->right+=rc->left;
	rc->bottom+=rc->top;
	
	blah:
	if(!p) {
		if(rc->left<-n) 
			rc->left=-n,rc->top-=(-rc->left-n)>>9<<3,rc->right=512-n;
		if(rc->top<-o) 
			rc->top=-o,rc->bottom=512-o;
		if(rc->right>512-n) 
			rc->left=-n,rc->bottom+=rc->right+n>>9<<3,rc->right=512-n;
		if(rc->bottom>512-o) 
			rc->top=-o,rc->bottom=512-o;
	}
}

void setobj(DUNGEDIT*ed,unsigned char*map){
	unsigned char c=0;
	short k,l,m,n,o;
	unsigned char*rom;
	dm_x&=0xfff;
	dm_l&=0xf;
	o=map[2];
	if(dm_k>0xff) {
		if((dm_x&0xf3f)==0xf3f) 
			goto invalobj;
		map[0]=0xfc+((dm_x>>4)&3);
		map[1]=(dm_x<<4)+(dm_x>>8);
		map[2]=(dm_x&0xc0)|dm_k;
	}
	else {
		if((dm_x&0x3f)==0x3f) 
			goto invalobj;
		if(dm_k<0xf8) {
			if((dm_l==3||!dm_l) && dm_x==0xffc) {
				invalobj:
				if(ed->withfocus&10) 
					ed->withfocus|=4;
				else MessageBox(framewnd,"You cannot place that object there","No",MB_OK);
					getobj(map);
				return;
			}
			map[0]=(dm_x<<2)|(dm_l>>2);
			map[1]=((dm_x>>4)&0xfc)|(dm_l&3);
		}
		else {
			if((dm_l==12||!dm_l) && dm_x==0xffc) 
				goto invalobj;
			if((dm_k==0xf9 && dm_l==9) || (dm_k==0xfb && dm_l==1)) 
				c=1;
			map[0]=(dm_x<<2)|(dm_l&3);
			map[1]=((dm_x>>4)&0xfc)|(dm_l>>2);
		}
		map[2]=(unsigned char)dm_k;
	}
	if(c && !ed->ischest) {
		rom=ed->ewdoc->rom;
		m=0;
		for(l=0;l<0x1f8;l+=3)
			if(*(short*)(rom+0xe96e+l)==-1) 
				break;
		if(l!=0x1f8) {
			for(k=0;k<0x1f8;k+=3) {
				n=*(short*)(rom+0xe96e+l);
				if(n==ed->mapnum) {
					if(ed->chestloc[m++]>map-ed->buf) {
						if(l<k) 
							MoveMemory(rom+0xe96e+l,rom+0xe96e+l+3,k-l-3);
						else 
							MoveMemory(rom+0xe96e+k+3,rom+0xe96e+k,l-k-3);
						setchest:*(short*)(rom+0xe96e+k)=ed->mapnum|((dm_k==0xfb)?0x8000:0);
						rom[0xe970+k]=0;
						break;
					}
				}
			}
			if(k==0x1f8) {
				k=l;
				goto setchest;
			}
		}
	}
	else if(ed->ischest && (map[2]!=o || !c)) {
		for(k=0;k<ed->chestnum;k++) 
			if(map-ed->buf==ed->chestloc[k]) 
				break;
			for(l=0;l<0x1f8;l+=3) {
				if((*(short*)(ed->ewdoc->rom+0xe96e+l)&0x7fff)==ed->mapnum) {
					k--;
					if(k<0) {
						*(short*)(ed->ewdoc->rom+0xe96e+l)=c?(ed->mapnum+((o==0xf9)?32768:0)):-1;
						break;
					}
				}
			}
	}
	if(ed->withfocus&4) {
		SetCursor(normal_cursor);
		ed->withfocus&=-5;
	}
	ed->modf=1;
}


void getdoor(unsigned char*map,unsigned char*rom){
	unsigned short i;
	i=*(unsigned short*)map;
	dm_dl=(i&0xf0)>>4;
	dm_l=i>>9;
	dm_k=i&3;
	if(dm_l==24 && !dm_k)
		dm_x=(((unsigned short*)(rom+0x19de))[dm_dl])>>1;
	else 
		dm_x=(((unsigned short*)(rom+0x197e))[dm_dl+dm_k*12])>>1;
}

void setdoor(unsigned char*map){
	dm_k&=3;
	map[0]=dm_k+(dm_dl<<4);
	map[1]=dm_l<<1;
}

unsigned char*Drawmap(unsigned char*rom,unsigned short*nbuf,unsigned char*map,DUNGEDIT*ed){
	unsigned short i;
	unsigned char l,m,o;
	short n;
	unsigned char ch=ed->chestnum;
	unsigned short*dm_src,*tmp;
	for(;;){
		i=*(unsigned short*)map;
		if(i==0xffff) 
			break;
		if(i==0xfff0) {
			for(;;) {
				map+=2;
				i=*(unsigned short*)(map);
				if(i==0xffff) 
					goto end;
				getdoor(map,rom);
				dm_wr=nbuf+dm_x;
				switch(dm_k) {
					case 0:
					switch(dm_l) {
						case 24:
						dm_l=42;
						dm_rd=(short*)(rom+0x1b52+((unsigned short*)(rom+0x4e06))[dm_l]);
						drawhole(18,nbuf);
						dm_rd=(short*)(rom+0x1b52+((unsigned short*)(rom+0x4d9e))[dm_l]);
						dm_x+=364;
						drawhole(18,nbuf);
						break;
						case 11:
						case 10:
						case 9:
						break;
						case 25:
						dm_rd=(short*)(rom+0x22dc);
						drawXx4(4);
						break;
						case 3:
						tofrontv(dm_x+64);
						break;
						case 1:
						tofront4x7(dm_x&0x783f);
						default:
						if(dm_l<0x20) {
							if(dm_dl>5) {
								dm_wr=nbuf+((((unsigned short*)(rom+0x198a))[dm_dl])>>1);
								if(dm_l==1)
									tofront4x7(dm_x+256);
								dm_rd=(short*)(rom+0x1b52+((unsigned short*)(rom+0x4e06))[dm_l]);
								m=4;
								while(m--) {
									dm_wr[64]=dm_rd[0];
									dm_wr[128]=dm_rd[1];
									dm_wr[192]=dm_rd[2];
									dm_rd+=3;
									dm_wr++;
								}
							}
							n=0;
							dm_rd=(short*)(rom+0x1b52+((unsigned short*)(rom+0x4d9e))[dm_l]);
							dm_wr=nbuf+dm_x;
							m=4;
							while(m--) {
								dm_wr[0]=dm_rd[0];
								dm_wr[64]=dm_rd[1];
								dm_wr[128]=dm_rd[2];
								dm_rd+=3;
								dm_wr++;
							}
						}
						else {
							n=dm_x;
							if(dm_dl>5 && dm_l!=0x23) {
								dm_x=(((unsigned short*)(rom+0x198a))[dm_dl])>>1;
								drawadd4(rom,dm_x);
							}
							dm_x=n;
							dm_rd=(short*)(rom+0x1b52+((unsigned short*)(rom+0x4d9e))[dm_l]);
							m=4;
							while(m--) {
								dm_buf[dm_x]=dm_rd[0];
								dm_buf[dm_x+0x1040]=dm_rd[1];
								dm_buf[dm_x+0x1080]=dm_rd[2];
								dm_rd+=3;
								dm_x++;
							}
							dm_x-=4;
							if(dm_l!=0x23) 
								tofrontv(dm_x);
						}
					}
					break;
					case 1:
					switch(dm_l) {
					case 3:
					while(dm_x&0x7c0) {
						dm_buf[dm_x]|=0x2000;
						dm_buf[dm_x+1]|=0x2000;
						dm_buf[dm_x+2]|=0x2000;
						dm_buf[dm_x+3]|=0x2000;
						dm_x+=64;
					}
					break;
					case 11: 
					case 10: 
					case 9:
					break;
					case 5: 
					case 6:
					dm_rd=(short*)(rom+0x41a8);
					o=10;
					l=8;
					dm_wr-=0x103;
					tmp=dm_wr;
					while(l--) {
						m=o;
						while(m--) 
							*(dm_wr++)=*(dm_rd++);
						tmp+=64;
						dm_wr=tmp;
					}
					break;
					case 2: 
					case 7: 
					case 8:
					tofront4x7(dm_x+0x100);
					dm_rd=(short*)(rom+0x4248);
					drawXx4(4);
					dm_x+=188;
					m=4;
					while(m--) 
						dm_buf[dm_x++]|=0x2000;
					break;
					case 1:
					tofront4x7(dm_x);
					default:
					dm_rd=(short*)(rom+0x1b52+((unsigned short*)(rom+0x4e06))[dm_l]);
					if(dm_l<0x20) {
						m=4;
						while(m--) {
							dm_wr[64]=dm_rd[0];
							dm_wr[128]=dm_rd[1];
							dm_wr[192]=dm_rd[2];
							dm_rd+=3;
							dm_wr++;
						}
					}
					else 
						drawadd4(rom,dm_x);
				}
				break;
				case 2:
				switch(dm_l) {
					case 10: 
					case 11:
					break;
					case 3:
					tofronth(dm_x+1);
					break;
					case 1:
					tofront5x4(dm_x&0x7fe0);
					default:
					if(dm_l<0x20) {
						if(dm_dl>5) {
							dm_wr=nbuf+((((unsigned short*)(rom+0x19ba))[dm_dl])>>1);
							dm_rd=(short*)(rom+0x1b52+((unsigned short*)(rom+0x4ec6))[dm_l]);
							m=3;
							dm_wr++;
							while(m--) {
								dm_wr[0]=dm_rd[0];
								dm_wr[64]=dm_rd[1];
								dm_wr[128]=dm_rd[2];
								dm_wr[192]=dm_rd[3];
								dm_rd+=4;
								dm_wr++;
							}
						}
						n=0;
						dm_rd=(short*)(rom+0x1b52+((unsigned short*)(rom+0x4e66))[dm_l]);
						dm_wr=nbuf+dm_x;
						m=3;
						while(m--) {
							dm_wr[0]=dm_rd[0];
							dm_wr[64]=dm_rd[1];
							dm_wr[128]=dm_rd[2];
							dm_wr[192]=dm_rd[3];
							dm_rd+=4;
							dm_wr++;
						}
					}
					else {
						if(dm_dl>5)
							drawaef0(rom,(((unsigned short*)(rom+0x19ba))[dm_dl])>>1);
						m=2;
						dm_rd=(short*)(rom+0x1b52+((unsigned short*)(rom+0x4e66))[dm_l]);
						dm_buf[dm_x]=dm_rd[0];
						dm_buf[dm_x+64]=dm_rd[1];
						dm_buf[dm_x+128]=dm_rd[2];
						dm_buf[dm_x+192]=dm_rd[3];
						while(m--) {
							dm_buf[dm_x+0x1001]=dm_rd[4];
							dm_buf[dm_x+0x1041]=dm_rd[5];
							dm_buf[dm_x+0x1081]=dm_rd[6];
							dm_buf[dm_x+0x10c1]=dm_rd[7];
							dm_rd+=4;
							dm_x++;
						}
					}
				}
				break;
				case 3:
				switch(dm_l) {
					case 10:
					case 11:
					break;
					case 3:
					dm_x+=2;
					while(dm_x&0x3f) {
						dm_buf[dm_x]|=0x2000;
						dm_buf[dm_x+64]|=0x2000;
						dm_buf[dm_x+128]|=0x2000;
						dm_buf[dm_x+192]|=0x2000;
						dm_x++;
					}
					break;
					case 1:
						tofront5x4(dm_x+4);
					default:
					if(dm_l<0x20) {
						dm_rd=(short*)(rom+0x1b52+((unsigned short*)(rom+0x4ec6))[dm_l]);
						dm_wr++;
						m=3;
						while(m--) {
							dm_wr[0]=dm_rd[0];
							dm_wr[64]=dm_rd[1];
							dm_wr[128]=dm_rd[2];
							dm_wr[192]=dm_rd[3];
							dm_rd+=4;
							dm_wr++;
						}
					}
					else 
						drawaef0(rom,dm_x);
				}
			}
		}
	}
	getobj(map);
	map+=3;
	dm_wr=nbuf+dm_x;
	if(dm_k>=0x100) {
		dm_src=dm_rd=(short*)(rom+0x1b52+((unsigned short*)(rom+0x81f0))[dm_k]);
		switch(dm_k-0x100) {
			case 59://dm_rd=(short*)(rom+0x2ce2);
			goto d43;
			case 58://dm_rd=(short*)(rom+0x2cca);
			goto d43;
			case 57://dm_rd=(short*)(rom+0x2cb2);
			goto d43;
			case 56://dm_rd=(short*)(rom+0x2c9a);
			d43:
			drawXx3(4);
			break;
			case 45: 
			case 46: 
			case 47:
			case 50: 
			case 51:
			case 0: 
			case 1: 
			case 2: 
			case 3: 
			case 4: 
			case 5: 
			case 6: 
			case 7:
			case 36: 
			case 37: 
			case 41: 
			case 28:
			drawXx4(4);
			break;
			case 48:
			case 49:
			case 8: 
			case 9: 
			case 10: 
			case 11: 
			case 12: 
			case 13: 
			case 14: 
			case 15:
			drawXx4bp(4);
			break;
			case 16: 
			case 17: 
			case 18: 
			case 19:
			drawXx4bp(3);
			break;
			case 20: 
			case 21: 
			case 22: 
			case 23:
			drawXx3bp(4);
			break;
			case 52:
			case 24: 
			case 25: 
			case 26: 
			case 27: 
			case 30: 
			case 31: 
			case 32:
			case 39:
			draw2x2();
			break;
			case 29: 
			case 33: 
			case 38: 
			case 43:
			drawXx3(2);
			break;
			case 34: 
			case 40:
			dm_l=5;
			while(dm_l--) {
				dm_wr[0]=dm_rd[0];
				dm_wr[1]=dm_rd[1];
				dm_wr[2]=dm_rd[2];
				dm_wr[3]=dm_rd[3];
				dm_rd+=4;
				dm_wr+=64;
			}
			break;
			case 35:
			drawXx3(4);
			break;
			case 42: 
			case 53:
			dm_l=1;
			draw4x2(1);
			break;
			case 62:
			case 44:d
			rawXx3(6);
			break;
			case 54:
			dm_rd=(short*)(rom+0x2c5a);
			dm_l=1;
			goto case99;
			case 55:
			drawXx4(10);
			break;
			case 60:
			n=*dm_rd;
			dm_l=6;
			while(dm_l--) {
				dm_buf[dm_x+1]=dm_buf[dm_x+5]=dm_buf[dm_x+9]=dm_buf[dm_x+15]=dm_buf[dm_x+19]=dm_buf[dm_x+23]=(dm_buf[dm_x]=dm_buf[dm_x+4]=dm_buf[dm_x+8]=dm_buf[dm_x+14]=dm_buf[dm_x+18]=dm_buf[dm_x+22]=dm_rd[0])|0x4000;
				dm_buf[dm_x+3]=dm_buf[dm_x+7]=dm_buf[dm_x+17]=dm_buf[dm_x+21]=(dm_buf[dm_x+2]=dm_buf[dm_x+6]=dm_buf[dm_x+16]=dm_buf[dm_x+20]=dm_rd[6])|0x4000;
				dm_rd++;
				dm_x+=64;
			}
			dm_rd+=6;
			dm_wr+=10;
			drawXx3(4);
			break;
			case 61:
			drawXx3(3);
			break;
			case 63:
			dm_l=8;
			while(dm_l--) {
				dm_buf[dm_x]=dm_rd[0];
				dm_buf[dm_x+0x40]=dm_rd[1];
				dm_buf[dm_x+0x80]=dm_rd[2];
				dm_buf[dm_x+0xc0]=dm_rd[3];
				dm_buf[dm_x+0x100]=dm_rd[4];
				dm_buf[dm_x+0x140]=dm_rd[5];
				dm_buf[dm_x+0x180]=dm_rd[6];
				dm_rd+=7;
			}
			break;
		}
	}
	else {
		if(dm_k>247) {
			dm_k&=7;
			dm_k<<=4;
			dm_k|=dm_l;
			dm_src=dm_rd=(short*)(rom+0x1b52+((unsigned short*)(rom+0x84f0))[dm_k]);
			switch(dm_k) {
				case 0:
				m=3;
				rows4:
				while(m--) {
					dm_wr[0]=dm_rd[0];
					dm_wr[1]=dm_rd[1];
					dm_wr[2]=dm_rd[2];
					dm_wr[3]=dm_rd[3];
					dm_rd+=4;
					dm_wr+=64;
				}
				break;
				case 1:
				m=5;
				goto rows4;
				case 2:
				m=7;
				goto rows4;
				case 3: 
				case 14:
				case 4: 
				case 5: 
				case 6: 
				case 7: 
				case 8: 
				case 9:
				case 10: 
				case 11: 
				case 12: 
				case 15:
				dm_wr[0]=dm_rd[0];
				break;
				case 13:
				dm_rd=(short*)(rom+0x2fda);
				case 23:
				m=5;
				tmp=dm_wr;
				while(m--) {
					dm_wr[2]=dm_wr[9]=dm_rd[1];
					dm_wr[73]=(dm_wr[66]=dm_rd[2])|0x4000;
					dm_wr[137]=(dm_wr[130]=dm_rd[4])|0x4000;
					dm_wr[201]=(dm_wr[194]=dm_rd[5])|0x4000;
					dm_wr++;
				}
				dm_wr=tmp;
				dm_wr[15]=(dm_wr[0]=dm_rd[0])|0x4000;
				dm_wr[14]=dm_wr[8]=dm_wr[7]=dm_wr[1]=dm_rd[1];
				dm_wr[142]=(dm_wr[129]=dm_rd[3])|0x4000;
				break;
				case 25:
				if(ch<16) 
					ed->chestloc[ch++]=map-ed->buf-3;
				case 16: 
				case 17: 
				case 19: 
				case 34: 
				case 35: 
				case 36:
				case 37:
				case 22: 
				case 24: 
				case 26: 
				case 47: 
				case 48: 
				case 62:
				case 63: 
				case 64: 
				case 65: 
				case 66: 
				case 67: 
				case 68: 
				case 69:
				case 70:
				case 73:
				case 74:
				case 79:
				case 80:
				case 81:
				case 82:
				case 83:
				case 86:
				case 87:
				case 88:
				case 89:
				case 94:
				case 95:
				case 99:
				case 100:
				case 101:
				case 117:
				case 124:
				case 125:
				case 126:
				draw2x2();
				break;
				case 18:
				m=3;
				while(m--) {
					dm_wr[384]=dm_wr[192]=dm_wr[0]=dm_rd[0];
					dm_wr[448]=dm_wr[256]=dm_wr[64]=dm_rd[1];
					dm_wr+=2;
				}
				break;
				case 20:
				drawXx3(4);
				break;
				case 21: 
				case 114:
				o=10;
				l=8;
				tmp=dm_wr;
				while(l--) {
					m=o;
					while(m--) *(dm_wr++)=*(dm_rd++);
						tmp+=64;
					dm_wr=tmp;
				}
				break;
				case 27: 
				case 28: 
				case 30: 
				case 31: 
				case 32:
				case 33:
				m=4;
				while(m--) {
					dm_buf[dm_x+0x1000]=dm_buf[dm_x]=dm_rd[0];
					dm_buf[dm_x+0x1040]=dm_buf[dm_x+0x40]=dm_rd[1];
					dm_buf[dm_x+0x1080]=dm_buf[dm_x+0x80]=dm_rd[2];
					dm_buf[dm_x+0x10c0]=dm_buf[dm_x+0xc0]=dm_rd[3];
					dm_x++;
					dm_rd+=4;
				}
				break;
				case 112:
				drawXx4(4);
				dm_wr+=124;
				dm_rd-=16;
				drawXx4(4);
				dm_wr+=252;
				drawXx4(4);
				break;
				case 113:
				drawXx4(4);
				drawXx4(4);
				dm_wr+=248;
				drawXx4(4);
				drawXx4(4);
				break;
				case 120:
				drawXx4(4);
				dm_wr+=250;
				drawXx4(4);
				dm_rd-=16;
				case 29: 
				case 102: 
				case 107:
				case 122:
				drawXx4(4);
				break;
				case 38: 
				case 39:
				m=4;
				while(m--) {
					dm_buf[dm_x+0x1000]=dm_buf[dm_x]=dm_rd[0];
					dm_buf[dm_x+0x1040]=dm_rd[1];
					dm_buf[dm_x+0x1080]=dm_rd[2];
					dm_buf[dm_x+0x10c0]=dm_rd[3];
					dm_x++;
					dm_rd+=4;
				}
				break;
				case 40: 
				case 41:
				m=4;
				while(m--) {
					dm_buf[dm_x+0x1000]=dm_rd[0];
					dm_buf[dm_x+0x1040]=dm_rd[1];
					dm_buf[dm_x+0x1080]=dm_rd[2];
					dm_buf[dm_x+0x10c0]=dm_buf[dm_x+0xc0]=dm_rd[3];
					dm_x++;
					dm_rd+=4;
				}
				break;
				case 42:
				dm_rd+=0xb6e;
				dm_x=0x20a;
				draw12x12();
				dm_rd-=3;
				dm_x=0x22a;
				draw12x12();
				dm_rd--;
				dm_x=0xa0a;
				draw12x12();
				dm_rd-=5;
				dm_x=0xa2a;
				draw12x12();
				break;
				case 44:
				draw2x2();
				dm_rd+=4;
				draw2x2();
				dm_wr+=124;
				dm_rd+=4;
				draw2x2();
				dm_rd+=4;
				case 43:
				draw2x2();
				break;
				case 45:
				dm_rd+=0xda5;
				m=14;
				while(m--) {
					dm_wr[13]=(dm_wr[0]=dm_rd[0])|0x4000;
					dm_wr[12]=dm_wr[11]=(dm_wr[2]=dm_wr[1]=dm_rd[14])^0x4000;
					dm_wr[10]=(dm_wr[3]=dm_rd[28])^0x4000;
					dm_wr[9]=(dm_wr[4]=dm_rd[42])^0x4000;
					dm_wr[8]=(dm_wr[5]=dm_rd[56])^0x4000;
					dm_wr[7]=(dm_wr[6]=dm_rd[70])^0x4000;
					dm_wr+=64;
					dm_rd++;
				}
				break;
				case 46:
				n=dm_x;
				m=6;
				dm_rd+=0xdf9;
				while(m--) {
					dm_buf[dm_x+0x107]=dm_buf[dm_x+0x10d]=dm_buf[dm_x+0x113]=dm_rd[0];
					dm_buf[dm_x+0x147]=dm_buf[dm_x+0x14d]=dm_buf[dm_x+0x153]=dm_rd[1];
					dm_buf[dm_x+0x187]=dm_buf[dm_x+0x18d]=dm_buf[dm_x+0x193]=dm_rd[2];
					dm_buf[dm_x+0x1c7]=dm_buf[dm_x+0x1cd]=dm_buf[dm_x+0x1d3]=dm_rd[3];
					dm_rd+=4;
					dm_x++;
				}
				m=5;
				dm_x=n;
				while(m--) {
					dm_buf[dm_x+0x117]=dm_buf[dm_x+0x158]=dm_buf[dm_x+0x199]=dm_buf[dm_x+0x1da]=dm_buf[dm_x+0x21b]=dm_buf[dm_x+0x25c]=dm_buf[dm_x+0x29d]=(dm_buf[dm_x+0x282]=dm_buf[dm_x+0x243]=dm_buf[dm_x+0x204]=dm_buf[dm_x+0x1c5]=dm_buf[dm_x+0x186]=dm_buf[dm_x+0x147]=dm_buf[dm_x+0x108]=dm_rd[0])|0x4000;
					dm_rd++;
					dm_x+=64;
				}
				m=6;
				dm_x=n;
				while(m--) {
					dm_buf[dm_x+0x2dd]=dm_buf[dm_x+0x45d]=dm_buf[dm_x+0x5dd]=(dm_buf[dm_x+0x2c2]=dm_buf[dm_x+0x442]=dm_buf[dm_x+0x5c2]=dm_rd[0])|0x4000;
					dm_buf[dm_x+0x2dc]=dm_buf[dm_x+0x45c]=dm_buf[dm_x+0x5dc]=(dm_buf[dm_x+0x2c3]=dm_buf[dm_x+0x443]=dm_buf[dm_x+0x5c3]=dm_rd[1])|0x4000;
					dm_buf[dm_x+0x2db]=dm_buf[dm_x+0x45b]=dm_buf[dm_x+0x5db]=(dm_buf[dm_x+0x2c4]=dm_buf[dm_x+0x444]=dm_buf[dm_x+0x5c4]=dm_rd[2])|0x4000;
					dm_buf[dm_x+0x2da]=dm_buf[dm_x+0x45a]=dm_buf[dm_x+0x5da]=(dm_buf[dm_x+0x2c5]=dm_buf[dm_x+0x445]=dm_buf[dm_x+0x5c5]=dm_rd[3])|0x4000;
					dm_rd+=4;
					dm_x+=64;
				}
				m=6;
				dm_x=n;
				while(m--) {
					dm_buf[dm_x+0x24c]=dm_buf[dm_x+0x252]=dm_rd[0];
					dm_buf[dm_x+0x28c]=dm_buf[dm_x+0x292]=dm_rd[6];
					dm_rd++;
					dm_x++;
				}
				dm_rd+=6;
				m=6;
				dm_x=n;
				while(m--) {
					dm_buf[dm_x+0x387]=dm_buf[dm_x+0x507]=dm_rd[0];
					dm_buf[dm_x+0x388]=dm_buf[dm_x+0x508]=dm_rd[1];
					dm_rd+=2;
					dm_x+=64;
				}
				m=5;
				dm_x=n;
				while(m--) {
					dm_buf[dm_x+0x247]=dm_rd[0];
					dm_buf[dm_x+0x287]=dm_rd[1];
					dm_buf[dm_x+0x2c7]=dm_rd[2];
					dm_buf[dm_x+0x307]=dm_rd[3];
					dm_buf[dm_x+0x347]=dm_rd[4];
					dm_rd+=5;
					dm_x++;
				}
				m=4;
				dm_x=n;
				while(m--) {
					dm_buf[dm_x+0x70e]|=0x2000;
					dm_buf[dm_x+0x74e]|=0x2000;
					dm_x++;
				}
				break;
				case 49:
				if(ch<16) 
					ed->chestloc[ch++]=map-ed->buf-3;
				case 50: 
				case 103: 
				case 104: 
				case 121:
				drawXx3(4);
				break;
				case 51: 
				case 72:
				drawXx4(4);
				break;
				case 52:
				case 53:
				case 56:
				case 57:
				draw3x2();
				break;
				case 54:
				case 55:
				case 77:
				case 93:
				drawXx3(6);
				break;
				case 58:
				case 59:
				drawXx3(4);
				dm_wr+=188;
				drawXx3(4);
				break;
				case 78:
				drawXx3(4);
				break;
				case 60:
				case 61: 
				case 92:
				drawXx4(6);
				break;
				case 71:
				draw2x2();
				dm_rd+=4;
				draw2x2();
				dm_rd+=4;
				dm_wr+=124;
				draw2x2();
				dm_rd+=4;
				draw2x2();
				break;
				case 75:
				case 118: 
				case 119:
				drawXx3(8);
				break;
				case 76:
				l=8;
				while(l--) {
					m=6;
					while(m--) {
						dm_wr[0]=dm_rd[0];
						dm_wr++;
						dm_rd++;
					}
					dm_wr+=58;
				}
				break;
				case 84:
				m=6;
				tmp=dm_wr;
				while(m--) {
					dm_wr[1]=dm_wr[2]=dm_wr[65]=dm_wr[66]=dm_rd[0];
					dm_wr[130]=(dm_wr[129]=dm_rd[1])|0x4000;
					dm_wr+=2;
				}
				dm_wr=tmp;
				m=3;
				while(m--) {
					dm_wr[0xc1]=dm_wr[0xc3]=dm_wr[0xcb]=dm_wr[0xcd]=(dm_wr[0xc0]=dm_wr[0xc2]=dm_wr[0xca]=dm_wr[0xcc]=dm_rd[2])|0x4000;
					dm_wr[0xc5]=dm_wr[0xc7]=dm_wr[0xc9]=(dm_wr[0xc4]=dm_wr[0xc6]=dm_wr[0xc8]=dm_rd[5])|0x4000;
					dm_rd++;
					dm_wr+=64;
				}
				dm_wr=tmp;
				dm_wr[77]=dm_wr[13]=(dm_wr[64]=dm_wr[0]=dm_rd[5])|0x4000;
				dm_wr[141]=(dm_wr[128]=dm_rd[6])|0x4000;
				m=4;
				dm_wr=tmp;
				dm_rd=dm_src;
				while(m--) {
					dm_wr[0x28a]=(dm_wr[0x283]=dm_rd[10])|0x4000;
					dm_wr[0x289]=(dm_wr[0x284]=dm_rd[14])|0x4000;
					dm_wr[0x288]=(dm_wr[0x285]=dm_rd[18])|0x4000;
					dm_wr[0x287]=(dm_wr[0x286]=dm_rd[22])|0x4000;
					dm_rd++;
					dm_wr+=64;
				}
				break;
				case 85: 
				case 91:
				m=3;
				dm_wr[0]=dm_rd[0];
				dm_wr[1]=dm_rd[1];
				dm_wr[2]=dm_rd[2];
				while(m--) {
					dm_wr[64]=dm_rd[3];
					dm_wr[65]=dm_rd[4];
					dm_wr[66]=dm_rd[5];
					dm_wr+=64;
				}
				dm_wr[64]=dm_rd[6];
				dm_wr[65]=dm_rd[7];
				dm_wr[66]=dm_rd[8];
				break;
				case 90:m=2;
				while(m--) {
					dm_wr[0]=dm_rd[0];
					dm_wr[1]=dm_rd[1];
					dm_wr[2]=dm_rd[2];
					dm_wr[3]=dm_rd[3];
					dm_rd+=4;
					dm_wr+=64;
				}
				break;
				case 96: 
				case 97:
				drawXx3(3);
				dm_wr+=189;
				drawXx3(3);
				break;
				case 98:
				m=22;
				while(m--) {
					dm_buf[dm_x+0x1000]=dm_rd[0];
					dm_buf[dm_x+0x1040]=dm_rd[1];
					dm_buf[dm_x+0x1080]=dm_rd[2];
					dm_buf[dm_x+0x10c0]=dm_rd[3];
					dm_buf[dm_x+0x1100]=dm_rd[4];
					dm_buf[dm_x+0x1140]=dm_rd[5];
					dm_buf[dm_x+0x1180]=dm_rd[6];
					dm_buf[dm_x+0x11c0]=dm_rd[7];
					dm_buf[dm_x+0x1200]=dm_rd[8];
					dm_buf[dm_x+0x1240]=dm_rd[9];
					dm_buf[dm_x+0x1280]=dm_rd[10];
					dm_rd+=11;
					dm_x++;
				}
				dm_x-=22;
				m=3;
				while(m--) {
					dm_buf[0x12c9+dm_x]=dm_rd[0];
					dm_buf[0x1309+dm_x]=dm_rd[3];
					dm_rd++;
					dm_x++;
				}
break;
case 105: case 106: case 110: case 111:drawXx4(3);
break;
case 109: case 108:drawXx3(4);
break;
case 115:fill4x2(rom,nbuf,dm_rd+0x70);
break;
case 123:tmp=dm_wr;
draw4x4X(5);
tmp+=256;
dm_wr=tmp;
draw4x4X(5);
break;
case 116:drawXx4(4);
drawXx4(4);
dm_wr+=248;
drawXx4(4);
drawXx4(4);
break;
}
continue;
}
dm_src=dm_rd=(short*)(rom+0x1b52+((unsigned short*)(rom+0x8000))[dm_k]);
switch(dm_k) {
case 0:len1();
while(dm_l--) draw2x2();
break;
case 1: case 2:len2();
while(dm_l--) drawXx4(2),dm_rd=dm_src;
break;
case 3:case 4:dm_l++;
while(dm_l--) {
dm_buf[0x1000+dm_x]=dm_buf[+dm_x]=dm_rd[0];
dm_buf[0x1040+dm_x]=dm_buf[0x40+dm_x]=dm_rd[1];
dm_buf[0x1080+dm_x]=dm_buf[0x80+dm_x]=dm_rd[2];
dm_buf[0x10c0+dm_x]=dm_buf[0xc0+dm_x]=dm_rd[3];
dm_buf[0x1001+dm_x]=dm_buf[0x1+dm_x]=dm_rd[4];
dm_buf[0x1041+dm_x]=dm_buf[0x41+dm_x]=dm_rd[5];
dm_buf[0x1081+dm_x]=dm_buf[0x81+dm_x]=dm_rd[6];
dm_buf[0x10c1+dm_x]=dm_buf[0xc1+dm_x]=dm_rd[7];
dm_x+=2;
}
break;
case 5: case 6:dm_l++;
while(dm_l--) drawXx4(2),dm_rd=dm_src,dm_wr+=4;
break;
case 7: case 8:dm_l++;
while(dm_l--) draw2x2();
break;
case 9: case 12: case 13: case 16: case 17: case 20:dm_l+=6;
while(dm_l--) draw1x5(),dm_wr-=63;
break;
case 21: case 24: case 25: case 28: case 29: case 32:n=-63;
case25:dm_l+=6;
while(dm_l--) {
dm_buf[dm_x+0x1000]=dm_buf[dm_x]=dm_rd[0];
dm_buf[dm_x+0x1040]=dm_buf[dm_x+0x40]=dm_rd[1];
dm_buf[dm_x+0x1080]=dm_buf[dm_x+0x80]=dm_rd[2];
dm_buf[dm_x+0x10c0]=dm_buf[dm_x+0xc0]=dm_rd[3];
dm_buf[dm_x+0x1100]=dm_buf[dm_x+0x100]=dm_rd[4];
dm_x+=n;
}
break;
case 23: case 26: case 27: case 30: case 31:n=65;
goto case25;
case 10: case 11: case 14: case 15: case 18: case 19: case 22:dm_l+=6;
while(dm_l--) draw1x5(),dm_wr+=65;
break;
case 33:dm_l=dm_l*2+1;
drawXx3(2);
while(dm_l--) dm_rd-=3,drawXx3(1);
drawXx3(1);
break;
case 34:dm_l+=2;
case34b:if(*dm_wr!=0xe2) *dm_wr=*dm_rd;
case34:dm_wr++;
dm_rd++;
while(dm_l--) *(dm_wr++)=*dm_rd;
dm_rd++;
*dm_wr=*dm_rd;
break;
case 35: case 36: case 37: case 38: case 39: case 40: case 41:case 42: case 43: case 44: case 45: case 46: case 179: case 180:dm_l++;
n=(*dm_wr)&0x3ff;
if(!(n==0x1db || n==0x1a6 || n==0x1dd || n==0x1fc))*dm_wr=*dm_rd;
goto case34;
case 47:dm_l+=10;
n=*(dm_rd++);
if(((*dm_wr)&0x3ff)!=0xe2) draw8fec(n);
dm_wr+=2;
dm_rd+=2;
while(dm_l--) {
dm_wr[0]=*dm_rd;
dm_wr[64]=n;
dm_wr++;
}
dm_rd++;
draw8fec(n);
break;
case 48:dm_l+=10;
n=*(dm_rd++);
if(((dm_wr[64])&0x3ff)!=0xe2) draw9030(n);
dm_wr+=2;
dm_rd+=2;
while(dm_l--) {
dm_wr[0]=n;
dm_wr[64]=*dm_rd;
dm_wr++;
}
dm_rd++;
draw9030(n);
break;
case 51:dm_l++;
while(dm_l--) dm_rd=dm_src,drawXx4(4);
break;
case 52:dm_l+=4;
n=*dm_rd;
while(dm_l--) *(dm_wr++)=n;
break;
case 53:break;
case 54: case 55:dm_l++;
while(dm_l--) dm_rd=dm_src,drawXx4(4),dm_wr+=2;
break;
case 56:dm_l++;
while(dm_l--) dm_rd=dm_src,drawXx3(2),dm_wr+=2;
break;
case 61:dm_l++;
while(dm_l--) dm_rd=dm_src,drawXx4(2),dm_wr+=4;
break;
case 57:dm_l++;
while(dm_l--) dm_rd=dm_src,drawXx4(2),dm_wr+=4;
break;
case 58: case 59:dm_l++;
while(dm_l--) dm_rd=dm_src,drawXx3(4),dm_wr+=4;
break;
case 60:dm_l++;
while(dm_l--) {
dm_rd=dm_src,draw2x2();
dm_wr+=0x17e;
dm_rd+=4;
draw2x2();
dm_wr-=0x17e;
}
break;
case 62: case 75:dm_l++;
while(dm_l--) draw2x2(),dm_wr+=12;
break;
case 63: case 64: case 65: case 66: case 67: case 68: case 69:case 70:dm_l++;
n=(*dm_wr)&0x3ff;
if(!(n==0x1db || n==0x1a6 || n==0x1dd || n==0x1fc)) *dm_wr=*dm_rd;
dm_rd++;
dm_wr++;
n=*(dm_rd++);
while(dm_l--) *(dm_wr++)=n;
*dm_wr=*dm_rd;
break;
case 71:dm_l++;
dm_l<<=1;
draw1x5();
dm_rd+=5;
dm_wr++;
while(dm_l--) draw1x5(),dm_wr++;
dm_rd+=5;
draw1x5();
break;
case 72:dm_l++;
dm_l<<=1;
drawXx3(1);
while(dm_l--) {
dm_wr[0]=dm_rd[0];
dm_wr[64]=dm_rd[1];
dm_wr[128]=dm_rd[2];
dm_wr++;
}
dm_rd+=3;
drawXx3(1);
break;
case 73: case 74:dm_l++;
draw4x2(4);
break;
case 76:dm_l++;
dm_l<<=1;
drawXx3(1);
while(dm_l--) drawXx3(1),dm_rd-=3;
dm_rd+=3;
drawXx3(1);
break;
case 77: case 78: case 79:dm_l++;
drawXx4(1);
while(dm_l--) drawXx4(2),dm_rd-=8;
dm_rd+=8;
drawXx4(1);
break;
case 80:dm_l+=2;
n=*dm_rd;
while(dm_l--) *(dm_wr++)=n;
break;
case 81: case 82: case 91: case 92:drawXx3(2);
while(dm_l--) drawXx3(2),dm_rd-=6;
dm_rd+=6;
drawXx3(2);
break;
case 83:dm_l++;
while(dm_l--) draw2x2();
break;
case 85: case 86:dm_l++;
draw4x2(12);
break;
case 93:dm_l+=2;
drawXx3(2);
while(dm_l--) {
dm_wr[0]=dm_rd[0];
dm_wr[64]=dm_rd[1];
dm_wr[128]=dm_rd[2];
dm_wr++;
}
dm_rd+=3;
drawXx3(2);
break;
case 94:dm_l++;
while(dm_l--) draw2x2(),dm_wr+=2;
break;
case 95:dm_l+=21;
goto case34b;
case 96: case 146: case 147:len1();
while(dm_l--) draw2x2(),dm_wr+=126;
break;
case 97: case 98:len2();
draw4x2(0x80);
break;
case 99: case 100:dm_l++;
case99:while(dm_l--) {
dm_buf[0x1000+dm_x]=dm_buf[dm_x]=dm_rd[0];
dm_buf[0x1001+dm_x]=dm_buf[1+dm_x]=dm_rd[1];
dm_buf[0x1002+dm_x]=dm_buf[2+dm_x]=dm_rd[2];
dm_buf[0x1003+dm_x]=dm_buf[3+dm_x]=dm_rd[3];
dm_buf[0x1040+dm_x]=dm_buf[0x40+dm_x]=dm_rd[4];
dm_buf[0x1041+dm_x]=dm_buf[0x41+dm_x]=dm_rd[5];
dm_buf[0x1042+dm_x]=dm_buf[0x42+dm_x]=dm_rd[6];
dm_buf[0x1043+dm_x]=dm_buf[0x43+dm_x]=dm_rd[7];
dm_x+=128;
}
break;
case 101: case 102:dm_l++;
draw4x2(0x180);
break;
case 103: case 104:dm_l++;
while(dm_l--) draw2x2(),dm_wr+=126;
break;
case 105:dm_l+=2;
if((*dm_wr&0x3ff)!=0xe3) *dm_wr=*dm_rd;
dm_wr+=64;
while(dm_l--) *dm_wr=dm_rd[1],dm_wr+=64;
*dm_wr=dm_rd[2];
break;
case 106: case 107: case 121: case 122:dm_l++;
while(dm_l--) *dm_wr=*dm_rd,dm_wr+=64;
break;
case 108:dm_l+=10;
n=*(dm_rd++);
if((*dm_wr&0x3ff)!=0xe3) draw9078(n);
dm_rd+=2;
dm_wr+=128;
while(dm_l--) *dm_wr=*dm_rd,dm_wr[1]=n,dm_wr+=64;
dm_rd++;
draw9078(n);
break;
case 109:dm_l+=10;
n=*(dm_rd++);
if((dm_wr[1]&0x3ff)!=0xe3) draw90c2(n);
dm_rd+=2;
dm_wr+=128;
while(dm_l--) *dm_wr=n,dm_wr[1]=*dm_rd,dm_wr+=64;
dm_rd++;
draw90c2(n);
break;
case 112:dm_l++;
while(dm_l--) dm_rd=dm_src,drawXx4(4),dm_wr+=252;
break;
case 113:dm_l+=4;
while(dm_l--) *dm_wr=*dm_rd,dm_wr+=64;
break;
case 115: case 116:dm_l++;
while(dm_l--) dm_rd=dm_src,drawXx4(4),dm_wr+=0x17c;
break;
case 117:dm_l++;
while(dm_l--) dm_rd=dm_src,drawXx4(2),dm_wr+=0x17e;
break;
case 118: case 119:dm_l++;
while(dm_l--) dm_rd=dm_src,drawXx4(3),dm_wr+=0x1fd;
break;
case 120: case 123:dm_l++;
while(dm_l--) dm_rd=dm_src,draw2x2(),dm_wr+=0x37e;
break;
case 124:dm_l+=2;
while(dm_l--) *dm_wr=*dm_rd,dm_wr+=64;
break;
case 125:dm_l++;
while(dm_l--) dm_rd=dm_src,draw2x2(),dm_wr+=0x7e;
break;
case 127: case 128:dm_l++;
while(dm_l--) dm_rd=dm_src,drawXx4(2),dm_wr+=0x2fe;
break;
case 129: case 130: case 131: case 132:dm_l++;
while(dm_l--) dm_rd=dm_src,drawXx4(3),dm_wr+=0x1fd;
break;
case 133: case 134:draw3x2(),dm_wr+=128,dm_rd+=6;
while(dm_l--) draw3x2(),dm_wr+=128;
dm_rd+=6;
draw3x2();
break;
case 135:dm_l++;
while(dm_l--) dm_rd=dm_src,drawXx4(2),dm_wr+=0x17e;
break;
case 136:dm_l++;
draw2x2(),dm_wr+=0x7e;
dm_rd+=4;
while(dm_l--) dm_wr[0]=dm_rd[0],dm_wr[1]=dm_rd[1],dm_wr+=64;
dm_rd+=2;
drawXx3(2);
break;
case 137:dm_l++;
while(dm_l--) dm_rd=dm_src,draw2x2(),dm_wr+=0xfe;
break;
case 138:dm_l+=21;
if((*dm_wr&0x3ff)!=0xe3) *dm_wr=*dm_rd;
dm_wr+=64;
while(dm_l--) *dm_wr=dm_rd[1],dm_wr+=64;
*dm_wr=dm_rd[2];
break;
case 140: case 139:dm_l+=8;
while(dm_l--) *dm_wr=*dm_rd,dm_wr+=64;
break;
case 141: case 142:dm_l++;
while(dm_l--) *dm_wr=*dm_rd,dm_wr+=64;
break;
case 143:dm_l+=2;
dm_l<<=1;
dm_wr[0]=dm_rd[0];
dm_wr[1]=dm_rd[1];
while(dm_l--) dm_wr[64]=dm_rd[2],dm_wr[65]=dm_rd[3],dm_wr+=64;
break;
case 144: case 145:len2();
draw4x2(0x80);
break;
case 148:dm_l++;
while(dm_l--) dm_rd=dm_src,drawXx4(4),dm_wr+=0xfc;
break;
case 149: case 150:dm_l++;
while(dm_l--) draw2x2(),dm_wr+=0x7e;
break;
case 160: case 169:dm_l+=4;
for(;
dm_l;
dm_l--) {
n=dm_l;
drawtr();
dm_wr+=64;
}
break;
case 161: case 166: case 170:dm_l+=4;
m=0;
while(dm_l--) {
m++;
n=m;
for(l=0;
l<n;
l++) dm_wr[l]=*dm_rd;
dm_wr+=64;
}
break;
case 162: case 167: case 171:dm_l+=4;
for(;
dm_l;
dm_l--) {
drawtr();
dm_wr+=65;
}
break;
case 163: case 168: case 172:dm_l+=4;
for(;
dm_l;
dm_l--) {
drawtr();
dm_wr-=63;
}
break;
case 164:dm_l+=4;
l=dm_l;
tmp=dm_wr;
while(l--) drawtr(),dm_src=dm_wr,dm_wr+=64;
dm_rd=(short*)(rom+0x218e);
m=2;
dm_wr=tmp;
while(m--) {
l=dm_l-2;
dm_wr[0]=dm_rd[0];
while(l--) dm_wr[1]=dm_rd[1],dm_wr++;
dm_wr[1]=dm_rd[2];
dm_rd+=3;
dm_wr=dm_src;
}
dm_wr=tmp+64;
m=dm_l-1;
l=m-1;
dm_wr+=m;
dm_src=dm_wr;
dm_wr=tmp+64;
m=2;
dm_rd=(short*)(rom+0x219a);
while(m--) {
n=l;
while(n--) *dm_wr=*dm_rd,dm_wr+=64;
dm_wr=dm_src;
dm_rd++;
}
break;
case 165:dm_l+=4;
for(;
dm_l;
dm_l--) drawtr(),dm_wr+=64;
break;
case 176: case 177:dm_l+=8;
drawtr();
break;
case 178:dm_l++;
while(dm_l--) dm_rd=dm_src,drawXx4(4);
break;
case 181:dm_l++;
 goto c182;
case 182: case 183:len2();
c182:while(dm_l--) drawXx4(2),dm_rd-=8;
break;
case 184: // yeah!len2();
while(dm_l--) draw2x2();
break;
case 185:len1();
while(dm_l--) draw2x2();
break;
case 186:dm_l++;
while(dm_l--) dm_rd=dm_src,drawXx4(4);
break;
case 187:dm_l++;
while(dm_l--) draw2x2(),dm_wr+=2;
break;
case 188: case 189:dm_l++;
while(dm_l--) draw2x2();
break;
case 192: case 194:l=(dm_l&3)+1;
dm_l>>=2;
dm_l++;
while(l--) {
m=dm_l;
dm_src=dm_wr;
while(m--)dm_wr[0]=dm_wr[1]=dm_wr[2]=dm_wr[3]=dm_wr[64]=dm_wr[65]=dm_wr[66]=dm_wr[67]=dm_wr[128]=dm_wr[129]=dm_wr[130]=dm_wr[131]=dm_wr[192]=dm_wr[193]=dm_wr[194]=dm_wr[195]=*dm_rd,dm_wr+=4;
dm_wr=dm_src+256;
}
break;
case 193:l=(dm_l&3)+1;
dm_l>>=2;
dm_l+=4;
tmp=dm_wr;
m=dm_l;
drawXx3(3);
while(m--) {
drawXx3(2);
dm_rd-=6;
}
dm_rd+=6;
drawXx3(3);
dm_src=dm_rd;
tmp+=0xc0;
o=l;
while(o--) {
dm_wr=tmp;
dm_rd=dm_src;
m=dm_l;
draw3x2();
dm_rd+=6;
dm_wr+=3;
while(m--) draw2x2();
dm_rd+=4;
draw3x2();
tmp+=0x80;
}
dm_rd+=6;
dm_wr=tmp;
m=dm_l;
drawXx3(3);
while(m--) {
drawXx3(2);
dm_rd-=6;
}
dm_rd+=6;
drawXx3(3);
tmp-=(1+l)<<6;
dm_l+=2;
dm_wr=tmp+dm_l;
dm_rd=(short*)(rom+0x20e2);
draw2x2();
break;
case 195: case 215:l=(dm_l&3)+1;
dm_l>>=2;
dm_l++;
while(l--) {
tmp=dm_wr;
m=dm_l;
while(m--) {
dm_wr[0]=dm_wr[1]=dm_wr[2]=dm_wr[64]=dm_wr[65]=dm_wr[66]=dm_wr[128]=dm_wr[129]=dm_wr[130]=*dm_rd;
dm_wr+=3;
}
dm_wr=tmp+0xc0;
}
break;
00098E65