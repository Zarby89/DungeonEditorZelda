[i]) {doc->fsize=lastaddr[i];break;}
		for(i = 0; i < 31; i++)
			((int*) (rom + 0x100004))[i] = lastaddr[i+8];
	}

	if(move_flag[v]) memmove(rom + r,rom + s,p - s);
	
	doc->rom = rom;
	
	for(i = 0;i < max; i++)
	{
		m = l[i];
		q = u[i];
		
		if(q == 40)
			continue;
		
		if(m < base[q] || m > lastaddr[q] || (m > limit[q])) 
		{
			wsprintf(buffer,"Internal error, block %d is outside range %d",i,q);
			MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
		}
	}
nochange:
	
	for(i = 0;i < 320; i++) 
	{
		blah = (int*)(rom + 0x1794d + i*3);
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
	
	for(i = 0; i < 19; i++) 
	{
		blah = (int*) (rom + 0x26cc0 + i*3);
		
		*blah &= 0xff000000;
		*blah |= cpuaddr(l[i + 641]);
	}
	
	for(i = 0;i < 8; i++) 
	{
		blah=(int*) (rom + l[640] + i*3);
		*blah &= 0xff000000;
		*blah |= cpuaddr(l[i+660]);
	}
	
	for(i = 0; i < 7; i++) 
	{
		if(!(rom[0x1386 + i] & 128)) 
			continue;
		
		j = cpuaddr(l[i + 669]);
		rom[0x137d + i] = (unsigned char) j;
		rom[0x1386 + i] = (unsigned char) (j >> 8);
		rom[0x138f + i] = (unsigned char) (j >> 16);
	}

	for(i = 676; i < max; i++) 
	{
		if( l[i] != doc->segs[i-676] ) 
			doc->p_modf = 1;
		
		doc->segs[i-676] = l[i];
	}

	if(num < 4096) 
	{
		j = l[num];
		
		for(i = 0; i<320; i++) 
		{
			if(l[i + 320] == j) 
				t[i] = door_ofs;
		}
	}

	for(i = 0;i < 320; i++)
	{
		blah = (int*) (rom + 0xf8000 + i*3);
		*blah &= 0xff000000;
		*blah |= cpuaddr(l[i+320]);
		blah = (int*) (rom + 0xf83c0 + i*3);
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

unsigned char* GetBG3GFX(char *buf,int size)
{
	int c,d;

	unsigned char *buf2 = malloc(size);
	
	size >>= 1;
	
	for(d=0; d < size; d += 16) 
	{
		for(c = 0; c < 16; c++) 
			buf2[d+d+c]=buf[d+c];
		
		for(c = 0; c < 16; c++) 
			buf2[d+d+c+16]=0;	// interesting, it's interlaced with a row of zeroes every
								//other line.
	}

	return buf2;
}

//GetBG3GFX**************************************

//GetBlocks#*************************************

void Getblocks(FDOC *doc, int b)
{
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
	
	if(b == 225) 
	{
		buf = Unsnes(rom + 0x80000, 0x7000);
	} 
	else if(b == 224) 
	{
		buf2 = (char*) GetBG3GFX(rom + 0x70000, 0x2000);
		buf = Unsnes(buf2, 0x2000);
		free(buf2);
	} 
	else if(b == 223) 
	{
		buf = malloc(0x4000);
		memcpy(buf, rom + 0xc4000, 0x4000);
	} 
	else 
	{
		a = (rom[0x4f80 + b] << 16) & 0x00FF0000;
		a += (rom[0x505f + b] << 8) & 0x0000FF00;
		a += rom[0x513e + b] & 0x000000FF;
		a = romaddr(a);
		
		if(b >= 0x73 && b < 0x7f) 
		{
			buf2 = (char*) Make4bpp(rom + a, 0x800);
			buf = Unsnes(buf2, 0x800);
		} 
		else 
		{
			buf = (char*) Uncompress(rom + a,0,0);
			
			if(b >= 220 || (b >= 113 && b < 115)) 
			{
				buf2 = (char*) GetBG3GFX(buf, 0x1000);
				free(buf);
				
				buf = Unsnes(buf2, 0x1000);
			} 
			else 
			{
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

void Releaseblks(FDOC*doc,int b)
{
	ZBLOCKS *zbl = doc->blks + b;

	unsigned char  *buf, *rom, *b2;
	
	int a,c,d,e,f;
	
	if(b > 225 || b < 0) 
		return;
	
	zbl->count--;
	
	if(!zbl->count) 
	{
		if(zbl->modf) 
		{
			rom = doc->rom;
	
			if(b == 225) 
			{
				b2 = Makesnes(zbl->buf, 0xe000);
			
				memcpy(rom+0x80000, b2, 0x7000);
				free(b2);
				doc->modf = 1;
			} 
			else if(b == 224) 
			{
				b2 = Make2bpp(zbl->buf, 0x2000);
				memcpy(rom + 0x70000, b2, 0x1000);
				free(b2);
				doc->modf = 1;
			} 
			else if(b == 223) 
			{
				memcpy(rom + 0xc4000, zbl->buf, 0x4000);
				doc->modf = 1;
			} 
			else 
			{
				a=romaddr((rom[0x4f80 + b] << 16) + (rom[0x505f + b] << 8) + rom[0x513e + b]);
			
				if(b >= 0x73 && b < 0x7f) 
				{
					buf = Make3bpp(zbl->buf, 0x1000);
					memcpy(rom + a, buf, 0x600);
					doc->modf = 1;
				} 
				else 
				{
					if(b >= 220 || (b >= 113 && b < 115)) 
					{
						buf = Make2bpp(zbl->buf, 0x1000);
						b2 = Compress(buf, 0x800, &c, 0);
					} 
					else 
					{
						buf = Make3bpp(zbl->buf, 0x1000);
						b2=Compress(buf, 0x600, &c, 0);
					}
					
					free(buf);
					f = doc->gfxend;
					
					for(d = 0; d < 223; d++) 
					{
						e = romaddr( (rom[0x4f80 + d] << 16) + (rom[0x505f + d] << 8) + rom[0x513e + d]);
						if(e < f && e > a) 
							f = e;
					}
					
					if(doc->gfxend - f + a + c > 0xc4000) 
					{
						free(b2);
						wsprintf(buffer, "Not enough space for blockset %d", b);
						MessageBox(framewnd,buffer, "Bad error happened", MB_OK);
						
						goto done;
					}
					
					for(d = 0; d < 223; d++) 
					{
						e = romaddr( (rom[0x4f80 + d] << 16) + (rom[0x505f + d] << 8) + rom[0x513e + d]);
						
						if(e > a) 
						{
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
done:
		free(zbl->buf);
		zbl->buf = 0;
	}
}

//Releaseblks*****************************************

//LoadPal#**********************************

void Loadpal(void *ed, unsigned char *rom, int start, int ofs, int len, int pals)
{

	int i,
		j,
		k,
		l;
	
	short* a = (short*) (rom + romaddr(start));
	
	if( ( (DUNGEDIT*) ed)->hpal) 
	{
		PALETTEENTRY pe[16];
	
		for(i = 0; i < pals; ++i) 
		{
			for(j = 0; j < len; ++j) 
			{
				l = *(a++);
			
				pe[j].peRed = (char) ((l & 0x1f) << 3);
				pe[j].peGreen = (char) ((l & 0x3e0) >> 2);
				pe[j].peBlue = (char) ((l & 0x7c00) >> 7);
				pe[j].peFlags = 0;
			}
			
			SetPaletteEntries(( (DUNGEDIT*) ed)->hpal, (ofs + *(short*) (( (DUNGEDIT*) ed)->pal)) & 255, len, pe);
			
			ofs += 16;
		}
	}
	else
	{
		RGBQUAD* pal = ( (DUNGEDIT*) ed)->pal;
		
		for(i = 0; i < pals; ++i) 
		{
			k = ofs;
			
			for(j = 0; j < len; ++j) 
			{
				l = *(a++);
				
				pal[k].rgbRed = (char) ((l & 0x1f) << 3);
				pal[k].rgbGreen = (char) ((l & 0x3e0) >> 2);
				pal[k].rgbBlue = (char) ((l & 0x7c00) >> 7);
				pal[k].rgbReserved = 0;
				
				++k;
			}
			ofs += 16;
		}
	}
}



//LoadPal***********************************

void Changeselect(HWND hc,int sel)

{
	int sc;
	int i;
	RECT rc;
	OVEREDIT*ed;
	ed=(OVEREDIT*)GetWindowLong(hc,GWL_USERDATA);
	GetClientRect(hc,&rc);
	rc.top=((ed->sel_select>>2)-ed->sel_scroll)<<5;
	rc.bottom=rc.top+32;
	ed->selblk=sel;
	if(ed->schflag) {
		for(i=0;i<ed->schnum;i++) if(ed->schbuf[i]==sel) {
			sel=i;
			goto foundblk;
		}
		sel=-1;
	}
foundblk:
	ed->sel_select=sel;
	InvalidateRect(hc,&rc,0);
	if(sel==-1) return;
	sel>>=2;
	sc=ed->sel_scroll;
	if(sel>=sc+ed->sel_page) sc=sel+1-ed->sel_page;
	else if(sel<sc) sc=sel;
	SendMessage(hc,WM_VSCROLL,SB_THUMBPOSITION|(sc<<16),0);
	rc.top=(sel-ed->sel_scroll)<<5;
	rc.bottom=rc.top+32;
	InvalidateRect(hc,&rc,0);
}
const static short nxtmap[4]={-1,1,-16,16};
const static unsigned char bg2_ofs[]={
	0,32,64,96,128,160,192,224,1
};

//Initroom********************************

void Initroom(DUNGEDIT *ed, HWND win)
{
	unsigned char *buf2;
	
	int i,
		j,
		l,
		m;

	unsigned char *rom = ed->ew.doc->rom; // equate the romfile here with ed's.

	buf2 = rom + (ed->hbuf[1] << 2) + 0x75460;

//	Loadpal(ed,rom,0x1bd734+*((unsigned short*)(rom+0xdec4b+buf2[0])),0x21,15,6);
//  I didn't comment the above out, to the best of my knowledge, -MON

	j = 0x6073 + (ed->gfxtmp << 3); //
	
	ed->gfxnum = ed->hbuf[2]; 
	ed->sprgfx = ed->hbuf[3]; 
	ed->palnum = ed->hbuf[1]; 
	
	l = 0x5d97 + (ed->hbuf[2] << 2); // gives an offset (bunched in 4's)
	
	// determine blocsets 0-7
	for(i = 0; i < 8; i++)
		ed->blocksets[i] = rom[j++];
	
	// these are uniquely determined
	ed->blocksets[8] = (rom + 0x1011e) [ed->gfxnum];
	ed->blocksets[9] = 0x5c;
	ed->blocksets[10] = 0x7d;
	
	// rewrite blocksets 3-6, if necessary
	for(i = 3; i < 7; i++)
	{	
		m = rom[l++];

		if(m)
			ed->blocksets[i] = m;
	}

	l = 0x5c57 + (ed->sprgfx << 2);
	
	// determine blocksets 11-14, which covers them all.
	for(i = 0; i < 4; i++) 
		ed->blocksets[i + 11] = rom[l + i] + 0x73;
	
	//get the block graphics for our tilesets?
	for(i = 0; i < 15; i++)
		Getblocks(ed->ew.doc, ed->blocksets[i]);
	
	ed->layering = (unsigned char) (ed->hbuf[0] & 0xe1);

	// take bits 2-4, shift right 2 to get a 3 bit number.
	ed->coll = (unsigned char) ((ed->hbuf[0] & 0x1c) >> 2);

	ed->modf = ed->hmodf = 0; 
	// the header is unmodified, nor is the room, so far.
	// if something changes, the flag will be set.

	SetDlgItemInt(win, 3019, ed->buf[0] & 15, 0); // floor 1
	SetDlgItemInt(win, 3021, ed->buf[0] >> 4, 0); // floor 2
	SetDlgItemInt(win, 3041, ed->gfxnum, 0); //blockset
	SetDlgItemInt(win, 3043, ed->palnum, 0); //palette
}

//Initroom******************************

//LoadHeader

void LoadHeader(DUNGEDIT *ed,int map)
{
	// we are passed in a dungeon editing window, and a map number (room number)

	int i, // lower limit for the header offset.
		j, // counter variable for looping through all dungeon rooms.
		l, // size of the header
		m; // upper limit for he header offset.
	
	int headerAddrLocal = ed->ew.doc->headerLocation;

	unsigned char *rom = ed->ew.doc->rom;

	i = ( (short*) ( rom + headerAddrLocal) )[map];
	
	l = 14;
	
	// sort through all the other header offsets
	for(j = 0; j < 0x140; j++)
	{
		// m gives the upper limit for the header.
		// if is less than 14 bytes from i.
		m = ( (short*) (rom + headerAddrLocal))[j];
	
		if( (m > i) && (m < i + 14) )
		{
			l = m - i;
			break;
		}
	}

	ed->hsize = l; // determine the size of the header

	// FIX!!!!
	memcpy(ed->hbuf, rom + 0x28000 + i, 14); // copy 14 bytes from the i offset.
}

//LoadHeader********************************

//Openroom********************************

void Openroom(DUNGEDIT *ed, int map)
{
	int i,j,l;

	unsigned char *buf;
	unsigned short k;
	unsigned char *rom;
	
	HWND win = ed->dlg;
	
	ed->mapnum = map; // the room number, ranging from 0 to 295
	ed->ew.doc->dungs[map] = win;
	
	rom = ed->ew.doc->rom;
	
	// get the base address for this room.
	buf = rom + romaddr( *(int*) (rom + 0xf8000 + map * 3));
	
	i = 2; //we'll step in by 2 bytes here.
	
	ed->selobj=0;
	ed->selchk=0;
	
	for(j = 0; j < 6; j++) 
	{
		ed->chkofs[j] = i;
		
		for(;;) 
		{
			k = *(unsigned short*)(buf + i);
			
			if(k == 0xffff) // code indicating to stop.
				break;
			
			if(k == 0xfff0) // code indicating to do this loop...
			{
				j++;
				
				ed->chkofs[j] = i + 2;
				
				for(;;) 
				{
					i += 2;
					
					k = *(unsigned short*)(buf+i);
					
					if(k == 0xffff) 
						goto end;
					
					if(!ed->selobj)
						ed->selobj = i,
						ed->selchk = j;
				}
			} 
			else 
				i += 3;
			
			if(!ed->selobj) // if there is no selected object, pick one.
			{
				ed->selobj = i - 3,
				ed->selchk = j;
			}		
			
		}
		
		j++;
		
		ed->chkofs[j] = i+2;
end:
		i += 2;
	}

	// now we know where the room data separates into special subsections.
	
	j = ed->selchk & -2; // AND by 0xFFFFFFFE. Makes j into an even number.
	
	CheckDlgButton(win, 3034, j ? BST_UNCHECKED : BST_CHECKED);
	CheckDlgButton(win, 3035, (j == 2) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(win, 3036, (j == 4) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(win, 3046, (j == 6) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(win, 3057, (j == 7) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(win, 3058, (j == 8) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(win, 3059, (j == 9) ? BST_CHECKED : BST_UNCHECKED);
	// do some initial settings, determine which radio button to check first 1, 2, 3,...

	ed->len = i; // size of the buffer.
	ed->buf = malloc(i); // generate the buffer of size i.
	memcpy(ed->buf, buf, i);// copy the data from buf to ed->buf.
	
	SetDlgItemInt(win,3029, buf[1] >> 2, 0); // this is the "layout", ranging from 0-7
	
	LoadHeader(ed,map); // load the header information.
	
	Initroom(ed,win); // 

	wsprintf(buffer,"Room %d",map); // prints the room string in the upper left hander corner.
	
	SetDlgItemText(win, 3000, buffer); // ditto.
	
	if(map > 0xff)
	{
		// If we're in the upper range, certain buttons might be grayed out so we can't
		// get back to the lower range
		for(i = 0; i < 4; i++) 
		{
			EnableWindow(GetDlgItem(win, 3014 + i),((map + nxtmap[i]) & 0xff) < 0x28);
		}
	} 
	else 
	{
		EnableWindow(GetDlgItem(win,3014),1);
		EnableWindow(GetDlgItem(win,3015),1);
		EnableWindow(GetDlgItem(win,3016),1);
		EnableWindow(GetDlgItem(win,3017),1);
	}
	
	CheckDlgButton(win, 3034, BST_CHECKED);
	CheckDlgButton(win, 3035, BST_UNCHECKED);
	CheckDlgButton(win, 3036, BST_UNCHECKED);
	
	for(i = 0; i < 9; i++)
	{	
		if(ed->layering == bg2_ofs[i]) 
		{
			SendDlgItemMessage(win, 3039, CB_SETCURSEL, i, 0);
		
			break;
		}
	}
	
	SendDlgItemMessage(win, 3045, CB_SETCURSEL, ed->coll, 0);
	SetDlgItemInt(win, 3050, ed->sprgfx, 0);
	
	buf = rom + 0x50000 + ( (short*) (rom + ed->ew.doc->dungspr) )[map];
	
	for(i = 1; ; i += 3) 
	{
		if(buf[i] == 0xff)
			break;
	}
		
	ed->esize = i;
	ed->ebuf = malloc(i);
	
	memcpy(ed->ebuf,buf,i);
	CheckDlgButton(win, 3060, *ed->ebuf ? BST_CHECKED:BST_UNCHECKED);
	buf = rom + 0x10000 + ((short*) (rom + 0xdb69))[map];
	
	for(i = 0; ; i += 3)
		if(*(short*)(buf + i) == -1) break;
	
	ed->ssize = i;
	ed->sbuf = malloc(i);
	
	memcpy(ed->sbuf,buf,i);
	
	buf = rom + 0x2736a;
	
	l = *(short*)(rom + 0x88c1);
	
	for(i = 0; ; i += 2)
	{
		if(i >= l)
		{
			ed->tsize = 0;
			ed->tbuf = 0;
			
			break;
		}
		
		j = i;
		
		for( ; ; ) 
		{
			i += 2;
			
			if(*(short*)(buf + i)==-1)
				break;
		}
		
		if(*(short*)(buf + j) == map)
		{
			memcpy(ed->tbuf = malloc(ed->tsize = i - j),buf + j,i-j);
			
			break;
		}
	}
}

//OpenRoom*****************************

//SaveDungSecrets**********************

void Savedungsecret(FDOC*doc,int num,unsigned char*buf,int size)
{
	int i,j,k;
	int adr[0x140];
	unsigned char*rom=doc->rom;
	for(i=0;i<0x140;i++)
		adr[i]=0x10000+((short*)(rom+0xdb69))[i];
	j=adr[num];
	k=adr[num+1];
	if(*(short*)(rom+j)==-1) {
		if(!size) return;
		j+=size+2;
		adr[num]+=2;
	} else {
		if(!size) {
			if(j>0xdde9) {
				j-=2;
				adr[num]-=2;
			}
		} else j+=size;
	}
	if(*(short*)(rom+k)!=-1) k-=2;
	if(adr[0x13f]-k+j>0xe6b0) {
		MessageBox(framewnd,"Not enough space for secret items","Bad error happened",MB_OK);
		return;
	}
	memmove(rom+j,rom+k,adr[0x13f]+2-k);
	if(size) memcpy(rom+adr[num],buf,size);
	if(j==k) return;
	((short*)(rom+0xdb69))[num]=adr[num];
	for(i=num+1;i<0x140;i++) {
		((short*)(rom+0xdb69))[i]=adr[i]+j-k;
	}
}
int Savesprites(FDOC*doc,int num,unsigned char*buf,int size)
{
	int i,k,l,m,n;
	int adr[0x288];
	unsigned char*rom=doc->rom;
	if(size) size++;
	for(i=0;i<0x160;i++)
		adr[i]=((short*)(rom+0x4c881))[i]+0x50000;
	k=doc->dungspr-0x2c0;
	for(;i<0x288;i++)
		adr[i]=((short*)(rom+k))[i]+0x50000;
	if(num&65536) {
		num&=65535;
		l=adr[num];
		for(i=0;i<0x288;i++) if(adr[i]==l) {
			adr[num]=0x4cb41;
			goto nochg;
		}
		size=0;
	} else l=adr[num];
	if(l==0x4cb41 || l==doc->sprend-2) m=l=((num>=0x160)?(doc->dungspr+0x300):0x4cb42); else {
		m=(num>=0x160)?doc->sprend:doc->dungspr;
		for(i=0;i<0x288;i++) {
			n=adr[i];
			if(n<m && n>l) m=n;
		}
	}
	if(size==2 && !buf[0]) size=0;
	if(doc->sprend+l-m+size>0x4ec9f) {
		MessageBox(framewnd,"Not enough space for sprites","Bad error happened",MB_OK);
		return 1;
	}
	if(size+l!=m) {
		memmove(rom+l+size,rom+m,doc->sprend-m);
		doc->sprend+=size-m+l;
		for(i=0;i<0x288;i++) if(adr[i]>=m) adr[i]+=size-m+l;
	}
	if(!size) adr[num]=((num>=0x160)?(doc->sprend-2):0x4cb41); else {
		memcpy(rom+l,buf,size-1);
		rom[l+size-1]=0xff;
		adr[num]=l;
	}
	if(doc->dungspr>=l)
		doc->dungspr+=size-m+l;
nochg:
	for(i=0;i<0x160;i++)
		((short*)(rom+0x4c881))[i]=adr[i];
	k=doc->dungspr-0x2c0;
	for(;i<0x288;i++)
		((short*)(rom+k))[i]=adr[i];
	return 0;
}

//Saveroom******************************************

void Saveroom(DUNGEDIT *ed)
{
	unsigned char *rom = ed->ew.doc->rom; // get our romfile from the Dungedit struct.
	
	int headerAddrLocal = ed->ew.doc->headerLocation;
	
	int i,
		j,
		k,
		l,
		m,
		n;

	if( !ed->modf ) // if the dungeon map hasn't been modified, do nothing.
		return;
	
	// if it has... save it.
	if( ed->ew.param >= 0x8c) // if the map is an overlay... 
	{
		i = Changesize(ed->ew.doc, ed->ew.param + 0x301f5, ed->len-2);
		
		if(!i) 
			return;
		
		memcpy(rom + i, ed->buf + 2, ed->len - 2);
	} 
	else
	{
		Savesprites(ed->ew.doc, 0x160 + ed->mapnum, ed->ebuf, ed->esize);
	
		for(i = 5;i >= 1;i -= 2) 
		{	
			if(ed->chkofs[i] != ed->chkofs[i+1]) 
				break;
		}

		door_ofs = ed->chkofs[i];
	
		i = Changesize(ed->ew.doc, ed->mapnum + 0x30140, ed->len);
	
		if( !i )
			return;
	
		memcpy(rom + i, ed->buf, ed->len);
	
		Savedungsecret(ed->ew.doc, ed->mapnum, ed->sbuf, ed->ssize);
	
		k = *(short*)(rom + 0x88c1);
	
		if(*(short*)(rom + 0x2736a) == -1)
			k = 0;
	
		for(i = 0; i < k; i += 2)
		{
			j = i;
		
			for(;;)
			{
				j+=2;
				
				if(*(short*)(rom+0x2736a+j) == -1)
					break;
			}
		
			if(*(short*)(rom+0x2736a+i) == ed->mapnum)
			{
				if(!ed->tsize) j += 2;
			
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
	
		if(ed->tsize && i == k)
		{
			j = ed->tsize + 2;
		
			if(k + j > 0x120) 
noroom: 		MessageBox(framewnd,"Not enough room for torches","Bad error happened",MB_OK);
			else 
			{
				memcpy(rom + 0x2736a + k, ed->tbuf, ed->tsize);
				*(short*)(rom + 0x2736a + k + j - 2) = -1;
				k += j;
			}
		}
	
		if(!k) 
			k = 4, *(int*)(rom + 0x2736a) == -1;
	
		*(short*)(rom + 0x88c1) = k;
	
		if(m = ed->hmodf)// if the headers have been modified, save them.
		{
			i = 0x28000 + ((short*)(rom + headerAddrLocal))[ed->mapnum]; // some sort of lower bound.
			k = 0x28000 + *((short*)(rom + 0x27780)); // some sort of upper bound.
		
			for(j = 0; j < 0x140; j++)
			{
				if(j == ed->mapnum) // if we hit the map number we're currently on, keep moving.
					continue;
					
				if(0x28000 + ((short*)(rom + headerAddrLocal))[j] == i)
				{
					if(m > 1)
						goto headerok;
				
					wsprintf(buffer,"The room header of room %d is reused. Modify this one only?",ed->mapnum);
				
					if(MessageBox(framewnd,buffer,"Bad error happened",MB_YESNO)==IDYES)
					{
headerok:
					k = i;
					
					goto changeroom;
				
					}
				
					break;
			
				}
		
			}
		
			for(j = 0;j < 0x140; j++) 
			{
				l = 0x28000 + ((short*)(rom + headerAddrLocal))[j];
			
				if(l > i && l < k)
					k = l;
	
			}
changeroom:
		
			if(m > 1)
			{
				((short*)(rom + headerAddrLocal))[ed->mapnum] = ((short*)(rom + headerAddrLocal))[m-2];
			
				n = 0;
			} 
			else 
				n = ed->hsize;
		
			if(*((short*)(rom + 0x27780)) + i + n - k > 0)
				MessageBox(framewnd,"Not enough room for room header","Bad error happened",MB_OK);
			else 
			{
				memmove(rom + i + n,rom + k,0x28000 + *((short*)(rom + 0x27780)) - k);
				*((short*)(rom + 0x27780)) += i + n - k;
				memcpy(rom + i,ed->hbuf,n);
			
				for(j = 0; j < 0x140; j++)
				
					if(j != ed->mapnum && ((short*)(rom + headerAddrLocal))[j] + 0x28000 >= k)
						((short*)(rom + headerAddrLocal))[j] += i + n - k;
		
			}
		
			if(n) 
			{
				rom[i] = ed->layering|(ed->coll<<2);
				rom[i+1] = ed->palnum;
				rom[i+2] = ed->gfxnum;
				rom[i+3] = ed->sprgfx;
	
			}
	
		}
	
	}
	
	ed->modf=0;
	ed->ew.doc->modf=1;
}

//SaveRoom***********************************

//Closeroom*********************************

int Closeroom(DUNGEDIT *ed)
{
	int i;
	
	if(ed->ew.doc->modf != 2 && ed->modf) 
	{
		if(ed->ew.param < 0x8c)
			wsprintf(buffer,"Confirm modification of room %d?",ed->mapnum);
		else 
			wsprintf(buffer,"Confirm modification of overlay map?");
		
		switch(MessageBox(framewnd,buffer,"Dungeon editor",MB_YESNOCANCEL)) 
		{
		case IDYES:
			Saveroom(ed);
			
			break;
		
		case IDCANCEL:
			return 1;
		}
	}

	for(i = 0; i < 15; i++) 
		Releaseblks(ed->ew.doc,ed->blocksets[i]);
	
	if(ed->ew.param<0x8c) 
		ed->ew.doc->dungs[ed->mapnum] = 0;
	return 0;
}

//Closeroom**********************************

void fill4x2(unsigned char*rom,short*nbuf,short*buf)
{
	int i,j,k,l,m;
	for(l=0;l<4;l++) {
		i=((short*)(rom+0x1b02))[l]>>1;
		for(m=0;m<8;m++) {
			for(k=0;k<8;k++) {
				for(j=0;j<2;j++) {
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

void len1()
{
	if(!dm_l) dm_l=0x20;
}
void len2()
{
	if(!dm_l) dm_l=0x1a;
}
void draw2x2()
{
	dm_wr[0]=dm_rd[0];
	dm_wr[64]=dm_rd[1];
	dm_wr[1]=dm_rd[2];
	dm_wr[65]=dm_rd[3];
	dm_wr+=2;
}
void drawXx4(int x)
{
	while(x--) {
		dm_wr[0]=dm_rd[0];
		dm_wr[64]=dm_rd[1];
		dm_wr[128]=dm_rd[2];
		dm_wr[192]=dm_rd[3];
		dm_rd+=4;
		dm_wr++;
	}
}
void drawXx4bp(int x)
{
	while(x--) {
		dm_buf[0x1000+dm_x]=dm_buf[dm_x]=dm_rd[0];
		dm_buf[0x1040+dm_x]=dm_buf[0x40+dm_x]=dm_rd[1];
		dm_buf[0x1080+dm_x]=dm_buf[0x80+dm_x]=dm_rd[2];
		dm_buf[0x10c0+dm_x]=dm_buf[0xc0+dm_x]=dm_rd[3];
		dm_x++;
		dm_rd+=4;
	}
}
void drawXx3bp(int x)
{
	while(x--) {
		dm_buf[0x1000+dm_x]=dm_buf[dm_x]=dm_rd[0];
		dm_buf[0x1040+dm_x]=dm_buf[0x40+dm_x]=dm_rd[1];
		dm_buf[0x1080+dm_x]=dm_buf[0x80+dm_x]=dm_rd[2];
		dm_x++;
		dm_rd+=3;
	}
}
void drawXx3(int x)
{
	while(x--) {
		dm_wr[0]=dm_rd[0];
		dm_wr[64]=dm_rd[1];
		dm_wr[128]=dm_rd[2];
		dm_rd+=3;
		dm_wr++;
	}
}
void draw3x2()
{
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
void draw1x5()
{
	dm_wr[0]=dm_rd[0];
	dm_wr[64]=dm_rd[1];
	dm_wr[128]=dm_rd[2];
	dm_wr[192]=dm_rd[3];
	dm_wr[256]=dm_rd[4];
}
void draw8fec(short n)
{
	dm_wr[0]=dm_rd[0];
	dm_wr[1]=dm_rd[1];
	dm_wr[65]=dm_wr[64]=n;
}
void draw9030(short n)
{
	dm_wr[1]=dm_wr[0]=n;
	dm_wr[64]=dm_rd[0];
	dm_wr[65]=dm_rd[1];
}
void draw9078(short n)
{
	dm_wr[0]=dm_rd[0];
	dm_wr[64]=dm_rd[1];
	dm_wr[65]=dm_wr[1]=n;
}
void draw90c2(short n)
{
	dm_wr[64]=dm_wr[0]=n;
	dm_wr[1]=dm_rd[0];
	dm_wr[65]=dm_rd[1];
}

void draw4x2(int x)
{
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
void draw2x6(short*nbuf)
{
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
void drawhole(int l,short*nbuf)
{
	draw2x6(nbuf);
	dm_x+=2;
	dm_rd+=6;
	while(l--) {
		dm_buf[dm_x]=dm_buf[dm_x+64]=dm_buf[dm_x+128]=
		dm_buf[dm_x+192]=dm_buf[dm_x+256]=dm_buf[dm_x+320]=
		dm_rd[0];
		dm_x++;
	}
	draw2x6(nbuf);
}
void draw4x4X(int n)
{
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
void draw12x12()
{
	int m,l;
	l=12;
	while(l--) {
		m=12;
		while(m--) dm_buf[dm_x+0x1000]=dm_rd[0],dm_x++,dm_rd++;
		dm_x+=52;
	}
}
void draw975c()
{
	unsigned char m;
	m=dm_l;
	dm_wr[0]=dm_rd[0];
	while(m--) dm_wr[1]=dm_rd[3],dm_wr++;
	dm_wr[1]=dm_rd[6];
	dm_wr[2]=dm_wr[3]=dm_wr[4]=dm_wr[5]=dm_rd[9];
	m=dm_l;
	dm_wr[6]=dm_rd[12];
	while(m--) dm_wr[7]=dm_rd[15],dm_wr++;
	dm_wr[7]=dm_rd[18];
	dm_tmp+=64;
	dm_wr=dm_tmp;
}
void draw93ff()
{
	int m;
	short*tmp;
	m=dm_l;
	tmp=dm_wr;
	dm_wr[0]=dm_rd[0];
	while(m--) dm_wr[1]=dm_rd[1],dm_wr[2]=dm_rd[2],dm_wr+=2;
	dm_wr[1]=dm_rd[3];
	tmp+=64;
	dm_wr=tmp;
}

void drawtr()
{
	unsigned char n=dm_l,l;
	for(l=0;l<n;l++) dm_wr[l]=*dm_rd;
}

void tofront4x7(int n)
{
	int m=7;
	while(m--) {
		dm_buf[n]|=0x2000;
		dm_buf[n+1]|=0x2000;
		dm_buf[n+2]|=0x2000;
		dm_buf[n+3]|=0x2000;
		n+=64;
	}
}
void tofront5x4(int n)
{
	int m=5;
	while(m--) {
		dm_buf[n]|=0x2000;
		dm_buf[n+64]|=0x2000;
		dm_buf[n+128]|=0x2000;
		dm_buf[n+192]|=0x2000;
		n++;
	}
}
void tofrontv(int n)
{
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
void tofronth(int n)
{
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
void drawadd4(unsigned char*rom,int x)
{
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
void drawaef0(unsigned char*rom,int x)
{
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
	4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,4,4,4,4,
	2,2,2,2,4,2,2,2,2,2,4,4,4,4,2,2,4,4,4,2,6,4,4,4,
	4,4,4,4,2,4,4,10,4,4,4,4,24,3,6,1
};
const static char obj_h[64]={
	4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,3,3,3,3,
	2,2,2,2,4,3,2,2,2,3,5,3,4,4,3,2,5,4,2,3,3,4,4,4,
	4,4,4,4,2,2,2,4,3,3,3,3,6,3,3,7
};
const static char obj2_w[128]={
	4,4,4,1,1,1,1,1,1,1,1,1,1,16,1,1,
	2,2,5,2,4,10,2,16,2,2,2,4,4,4,4,4,
	4,4,2,2,2,2,4,4,4,4,44,2,4,14,28,2,
	2,4,4,4,3,3,6,6,3,3,4,4,6,6,2,2,
	2,2,2,2,2,2,2,4,4,2,2,8,6,6,4,2,
	2,2,2,2,14,3,2,2,2,2,4,3,6,6,2,2,
	3,3,22,2,2,2,4,4,4,3,3,4,4,4,3,3,
	4,8,10,64,8,2,8,8,8,4,4,20,2,2,2,1
};
const static char obj2_h[128]={
	3,5,7,1,1,1,1,1,1,1,1,1,1,4,1,1,
	2,2,8,2,3,8,2,4,2,2,2,4,4,4,4,4,
	4,4,2,2,2,2,4,4,4,4,44,2,4,14,25,2,
	2,3,3,4,2,2,3,3,2,2,6,6,4,4,2,2,
	2,2,2,2,2,2,2,4,4,2,2,3,8,3,3,2,
	2,2,2,2,14,5,2,2,2,2,2,5,4,3,2,2,
	6,6,13,2,2,2,4,3,3,4,4,4,3,3,4,4,
	10,8,8,64,8,2,3,3,8,3,4,8,2,2,2,1
};
const static char obj3_w[248]={
	0,0,0,0,0,-4,-4,0,0,5,5,5,5,5,5,5,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
	5,2,3,2,2,2,2,2,2,2,2,2,2,2,2,13,
	13,1,1,0,3,1,-2,-2,-2,-4,-4,-4,-2,-4,-12,2,
	2,2,2,2,2,2,2,2,2,0,0,-12,2,2,2,2,
	1,2,2,0,1,-8,-8,1,1,1,1,2,2,5,-2,22,
	2,4,4,4,4,4,4,2,2,1,1,1,2,2,1,1,
	4,1,1,4,4,2,3,3,2,1,1,2,1,2,1,2,
	2,3,3,3,3,3,3,2,2,2,1,1,1,1,1,2,
	4,4,2,2,4,2,2,1,1,1,1,1,1,1,1,1,
	3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,1,
	7,7,0,2,2,0,0,0,0,0,0,-2,0,0,1,1,
	0,12,0,0,0,0,0,0,0,0,0,1,1,3,3,1,
	1,0,0,1,1,1,1,0,4,0,4,0,8,2,0,0,
	0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1
};
const static char obj3_h[248]={
	2,4,4,4,4,4,4,2,2,9,9,9,9,9,9,9,
	9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
	9,3,1,1,1,1,1,1,1,1,1,1,1,1,1,2,
	2,1,1,4,1,1,4,4,3,4,3,3,8,4,2,1,
	1,1,1,1,1,1,1,5,3,2,2,2,3,4,4,4,
	1,3,3,2,1,2,2,1,1,1,1,3,3,3,2,1,
	0,0,0,0,0,-4,-4,0,0,3,0,0,13,13,1,1,
	0,3,1,-2,-2,-2,-4,-4,-12,0,0,-12,1,0,1,-8,
	-8,-4,-4,-4,-4,2,2,-2,5,-2,22,7,7,0,0,3,
	0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,
	3,3,3,3,3,3,3,3,3,3,3,3,3,1,1,1,
	1,1,4,1,1,4,4,4,2,2,4,2,2,2,1,1,
	0,6,0,0,0,0,0,0,0,0,0,1,1,0,0,1,
	1,0,0,1,1,1,1,0,4,0,4,0,5,2,0,0,
	0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1
};
const static char obj3_m[248]={
	2,2,2,2,2,6,6,2,2,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,0,0,4,1,0,6,6,4,6,8,8,4,6,14,1,
	1,1,1,1,1,1,1,2,2,4,4,14,2,2,2,2,
	1,2,2,2,0,12,12,0,0,0,0,2,2,1,4,1,
	2,2,2,2,2,6,6,2,2,1,1,1,1,1,0,0,
	4,1,0,6,6,6,8,8,14,1,1,14,1,2,0,12,
	12,8,8,8,8,2,2,6,1,4,1,1,1,1,1,2,
	2,2,2,2,4,2,2,0,0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,1,1,1,1,1,0,0,0,
	1,1,4,1,1,2,2,2,2,2,4,4,2,2,0,0,
	4,2,4,3,4,4,4,4,4,4,4,0,0,1,1,0,
	0,4,4,0,0,0,0,3,4,4,4,4,2,2,2,4,
	4,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0
};
const static unsigned char obj3_t[248]={
	0,2,2,1,1,1,1,1,1,97,65,65,97,97,65,65,
	97,97,65,65,97,97,65,65,97,97,65,65,97,97,65,65,
	97,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	128,130,130,129,129,129,129,129,129,129,129,129,129,129,129,129,
	129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,
	129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,129,
	130,130,128,128,129,129,129,129,129,129,129,129,129,129,129,129,
	65,65,65,113,65,65,65,65,113,65,65,65,113,129,129,129,
	1,1,1,1,1,1,2,2,2,2,1,1,1,1,1,1,
	3,3,3,3,3,3,3,3,3,3,3,1,1,20,4,1,
	1,3,3,1,1,1,1,3,3,3,3,3,3,3,3,3,
	3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0
};
void getobj(unsigned char*map)
{
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
	} else {
		dm_x=((j>>2)|((i&0xfc00u)>>4));
		dm_k=map[2];
		if(dm_k<0xf8)
		dm_l=((i&3)<<2)|((i&0x300u)>>8);
		else dm_l=(i&3)|((i&0x300u)>>6);
	}
}
char sprname[0x11c][16];
HDC objdc;
HBITMAP objbmp;

void Getstringobjsize(char*str,RECT*rc)
{
	GetTextExtentPoint(objdc,str,strlen(str),(LPSIZE)&(rc->right));
	rc->bottom++;
	rc->right++;
	if(rc->bottom<16) rc->bottom=16;
	if(rc->right<16) rc->right=16;
	rc->right+=rc->left;
	rc->bottom+=rc->top;
}
const static char obj4_h[4]={5,7,11,15};
const static char obj4_w[4]={8,16,24,32};
void Getdungobjsize(int chk,RECT*rc,int n,int o,int p)
{
	int a,b,c,d,e;
	char*f;
	switch(chk) {
	case 0: case 2: case 4:
		if(dm_k>=0x100) rc->right=obj_w[dm_k-0x100]<<3,rc->bottom=obj_h[dm_k-0x100]<<3;
		else if(dm_k>=0xf8) rc->right=obj2_w[(dm_k-0xf8<<4)+dm_l]<<3,rc->bottom=obj2_h[(dm_k-0xf8<<4)+dm_l]<<3;
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
				if(e&16) rc->left-=dm_l<<3;
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
			if(e&32) rc->top-=b+((e&16)?2:4)<<3;
			a+=obj3_w[dm_k];
			b+=obj3_h[dm_k];
			rc->right=a<<3;
			rc->bottom=b<<3;
			dm_l=d;
		}
		if(dm_k==0xff) {if(dm_l==8) rc->left-=16; else if(dm_l==3) rc->left=-n,rc->top=-o;}
		else if(dm_k==0xfa) {
			if(dm_l==14) rc->left+=16,rc->top+=32;
			else if(dm_l==10) rc->left=80-n,rc->top=64-o;
		}
		break;
	case 1: case 3: case 5:
		switch(dm_k) {
		case 0:
			if(dm_l==24) {
				rc->right=176;
				rc->bottom=96;
			} else if(dm_l==25) rc->right=rc->bottom=32; else if(dm_dl>8) {
				rc->top-=120;
				rc->bottom=144;
				rc->right=32;
			} else if(dm_dl>5) {
				rc->top-=72;
				rc->bottom=96;
				rc->right=32;
			} else rc->right=32,rc->bottom=24;
			break;
		case 1:
			if(dm_l==5 || dm_l==6) {
				rc->right=96;
				rc->bottom=64;
				rc->left-=32;
				rc->top-=32;
			} else if(dm_l==2 || dm_l==7 || dm_l==8) {
				rc->right=rc->bottom=32;
			} else rc->right=32,rc->bottom=24,rc->top+=8;
			break;
		case 2:
			rc->bottom=32;
			if(dm_dl>8) {
				rc->left-=104;
				rc->right=128;
			} else if(dm_dl>5) {
				rc->left-=56;
				rc->right=80;
			} else rc->right=24;
			break;
		case 3:
			rc->right=24,rc->bottom=32,rc->left+=8;
		}
		break;
	case 8: case 9:
		rc->right=16;
		rc->bottom=16;
		break;
	case 7:
		Getstringobjsize(cur_sec,rc);
		goto blah2;
	case 6:
		if(dm_k>=0x11c) f="Crash"; else f=sprname[dm_k];
		Getstringobjsize(f,rc);
blah2:
		if(rc->right>512-n) rc->right=512-n;
		if(rc->bottom>512-o) rc->bottom=512-o;
		goto blah;
	}
	rc->right+=rc->left;
	rc->bottom+=rc->top;
blah:
	if(!p) {
		if(rc->left<-n) rc->left=-n,rc->top-=(-rc->left-n)>>9<<3,rc->right=512-n;
		if(rc->top<-o) rc->top=-o,rc->bottom=512-o;
		if(rc->right>512-n) rc->left=-n,rc->bottom+=rc->right+n>>9<<3,rc->right=512-n;
		if(rc->bottom>512-o) rc->top=-o,rc->bottom=512-o;
	}
}
void setobj(DUNGEDIT*ed,unsigned char*map)
{
	unsigned char c=0;
	short k,l,m,n,o;
	unsigned char*rom;
	dm_x&=0xfff;
	dm_l&=0xf;
	o=map[2];
	if(dm_k>0xff) {
		if((dm_x&0xf3f)==0xf3f) goto invalobj;
		map[0]=0xfc+((dm_x>>4)&3);
		map[1]=(dm_x<<4)+(dm_x>>8);
		map[2]=(dm_x&0xc0)|dm_k;
	} else {
		if((dm_x&0x3f)==0x3f) goto invalobj;
		if(dm_k<0xf8) {
			if((dm_l==3||!dm_l) && dm_x==0xffc) {
invalobj:
				if(ed->withfocus&10) ed->withfocus|=4;
				else MessageBox(framewnd,"You cannot place that object there.","No",MB_OK);
				getobj(map);
				return;
			}
			map[0]=(dm_x<<2)|(dm_l>>2);
			map[1]=((dm_x>>4)&0xfc)|(dm_l&3);
		} else {
			if((dm_l==12||!dm_l) && dm_x==0xffc) goto invalobj;
			if((dm_k==0xf9 && dm_l==9) || (dm_k==0xfb && dm_l==1)) c=1;
			map[0]=(dm_x<<2)|(dm_l&3);
			map[1]=((dm_x>>4)&0xfc)|(dm_l>>2);
		}
		map[2]=(unsigned char)dm_k;
	}
	if(c && !ed->ischest) {
		rom=ed->ew.doc->rom;
		m=0;
		for(l=0;l<0x1f8;l+=3)
			if(*(short*)(rom+0xe96e+l)==-1) break;
		if(l!=0x1f8) {
			for(k=0;k<0x1f8;k+=3) {
				n=*(short*)(rom+0xe96e+l);
				if(n==ed->mapnum) {
					if(ed->chestloc[m++]>map-ed->buf) {
						if(l<k) MoveMemory(rom+0xe96e+l,rom+0xe96e+l+3,k-l-3);
						else MoveMemory(rom+0xe96e+k+3,rom+0xe96e+k,l-k-3);
setchest:
						*(short*)(rom+0xe96e+k)=ed->mapnum|((dm_k==0xfb)?0x8000:0);
						rom[0xe970+k]=0;
						break;
					}
				}
			}
			if(k==0x1f8) {k=l; goto setchest; }
		}
	} else if(ed->ischest && (map[2]!=o || !c)) {
		for(k=0;k<ed->chestnum;k++) if(map-ed->buf==ed->chestloc[k]) break;
		for(l=0;l<0x1f8;l+=3) {
			if((*(short*)(ed->ew.doc->rom+0xe96e+l)&0x7fff)==ed->mapnum) {
				k--;
				if(k<0) {
					*(short*)(ed->ew.doc->rom+0xe96e+l)=c?(ed->mapnum+((o==0xf9)?32768:0)):-1;
					break;
				}
			}
		}
	}
	if(ed->withfocus&4) {SetCursor(normal_cursor);ed->withfocus&=-5;}
	ed->modf=1;
}
void getdoor(unsigned char*map,unsigned char*rom)
{
	unsigned short i;
	i=*(unsigned short*)map;
	dm_dl=(i&0xf0)>>4;
	dm_l=i>>9;
	dm_k=i&3;
	if(dm_l==24 && !dm_k)
		dm_x=(((unsigned short*)(rom+0x19de))[dm_dl])>>1;
	else dm_x=(((unsigned short*)(rom+0x197e))[dm_dl+dm_k*12])>>1;
}
void setdoor(unsigned char*map)
{
	dm_k&=3;
	map[0]=dm_k+(dm_dl<<4);
	map[1]=dm_l<<1;
}
unsigned char*Drawmap(unsigned char*rom,unsigned short*nbuf,unsigned char*map,DUNGEDIT*ed)
{
	unsigned short i;
	unsigned char l,m,o;
	short n;
	unsigned char ch=ed->chestnum;
	unsigned short*dm_src,*tmp;
	for(;;)
	{
		i=*(unsigned short*)map;
		if(i==0xffff) break;
		if(i==0xfff0) {
			for(;;) {
				map+=2;
				i=*(unsigned short*)(map);
				if(i==0xffff) goto end;
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
					case 11: case 10: case 9:
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
						} else {
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
							if(dm_l!=0x23) tofrontv(dm_x);
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
					case 11: case 10: case 9:
						break;
					case 5: case 6:
						dm_rd=(short*)(rom+0x41a8);
						o=10;
						l=8;
						dm_wr-=0x103;
						tmp=dm_wr;
						while(l--) {
							m=o;
							while(m--) *(dm_wr++)=*(dm_rd++);
							tmp+=64;
							dm_wr=tmp;
						}
						break;
					case 2: case 7: case 8:
						tofront4x7(dm_x+0x100);
						dm_rd=(short*)(rom+0x4248);
						drawXx4(4);
						dm_x+=188;
						m=4;
						while(m--) dm_buf[dm_x++]|=0x2000;
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
						} else drawadd4(rom,dm_x);
					}
					break;
				case 2:
					switch(dm_l) {
					case 10: case 11:
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
						} else {
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
					case 10: case 11:
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
						} else drawaef0(rom,dm_x);
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
			case 59:
//				dm_rd=(short*)(rom+0x2ce2);
				goto d43;
			case 58:
//				dm_rd=(short*)(rom+0x2cca);
				goto d43;
			case 57:
//				dm_rd=(short*)(rom+0x2cb2);
				goto d43;
			case 56:
//				dm_rd=(short*)(rom+0x2c9a);
d43:
				drawXx3(4);
				break;
			case 45: case 46: case 47:
			case 50: case 51:
			case 0: case 1: case 2: case 3: case 4: case 5: case 6: case 7:
			case 36: case 37: case 41: case 28:
				drawXx4(4);
				break;
			case 48: case 49:
			case 8: case 9: case 10: case 11: case 12: case 13: case 14: case 15:
				drawXx4bp(4);
				break;
			case 16: case 17: case 18: case 19:
				drawXx4bp(3);
				break;
			case 20: case 21: case 22: case 23:
				drawXx3bp(4);
				break;
			case 52:
			case 24: case 25: case 26: case 27: case 30: case 31: case 32:
			case 39:
				draw2x2();
				break;
			case 29: case 33: case 38: case 43:
				drawXx3(2);
				break;
			case 34: case 40:
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
			case 42: case 53:
				dm_l=1;
				draw4x2(1);
				break;
			case 62:
			case 44:
				drawXx3(6);
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
					dm_buf[dm_x+1]=dm_buf[dm_x+5]=dm_buf[dm_x+9]=
					dm_buf[dm_x+15]=dm_buf[dm_x+19]=dm_buf[dm_x+23]=
					(dm_buf[dm_x]=dm_buf[dm_x+4]=dm_buf[dm_x+8]=
					dm_buf[dm_x+14]=dm_buf[dm_x+18]=dm_buf[dm_x+22]=dm_rd[0])|0x4000;
					dm_buf[dm_x+3]=dm_buf[dm_x+7]=dm_buf[dm_x+17]=
					dm_buf[dm_x+21]=
					(dm_buf[dm_x+2]=dm_buf[dm_x+6]=dm_buf[dm_x+16]=
					dm_buf[dm_x+20]=dm_rd[6])|0x4000;
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
		} else {
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
				case 3: case 14:
				case 4: case 5: case 6: case 7: case 8: case 9:
				case 10: case 11: case 12: case 15:
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
					if(ch<16) ed->chestloc[ch++]=map-ed->buf-3;
				case 16: case 17: case 19: case 34: case 35: case 36:
				case 37:
				case 22: case 24: case 26: case 47: case 48: case 62:
				case 63: case 64: case 65: case 66: case 67: case 68: case 69:
				case 70: case 73: case 74: case 79: case 80: case 81: case 82:
				case 83: case 86: case 87: case 88: case 89: case 94:
				case 95: case 99: case 100: case 101: case 117: case 124:
				case 125: case 126:
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
				case 21: case 114:
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
				case 27: case 28: case 30: case 31: case 32:
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
				case 29: case 102: case 107:
				case 122:
					drawXx4(4);
					break;
				case 38: case 39:
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
				case 40: case 41:
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
						dm_wr[12]=dm_wr[11]=
						(dm_wr[2]=dm_wr[1]=dm_rd[14])^0x4000;
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
						dm_buf[dm_x+0x117]=dm_buf[dm_x+0x158]=dm_buf[dm_x+0x199]=
						dm_buf[dm_x+0x1da]=dm_buf[dm_x+0x21b]=dm_buf[dm_x+0x25c]=dm_buf[dm_x+0x29d]=
						(dm_buf[dm_x+0x282]=dm_buf[dm_x+0x243]=dm_buf[dm_x+0x204]=
						dm_buf[dm_x+0x1c5]=dm_buf[dm_x+0x186]=dm_buf[dm_x+0x147]=dm_buf[dm_x+0x108]=dm_rd[0])|0x4000;
						dm_rd++;
						dm_x+=64;
					}
					m=6;
					dm_x=n;
					while(m--) {
						dm_buf[dm_x+0x2dd]=dm_buf[dm_x+0x45d]=dm_buf[dm_x+0x5dd]=
						(dm_buf[dm_x+0x2c2]=dm_buf[dm_x+0x442]=dm_buf[dm_x+0x5c2]=dm_rd[0])|0x4000;
						dm_buf[dm_x+0x2dc]=dm_buf[dm_x+0x45c]=dm_buf[dm_x+0x5dc]=
						(dm_buf[dm_x+0x2c3]=dm_buf[dm_x+0x443]=dm_buf[dm_x+0x5c3]=dm_rd[1])|0x4000;
						dm_buf[dm_x+0x2db]=dm_buf[dm_x+0x45b]=dm_buf[dm_x+0x5db]=
						(dm_buf[dm_x+0x2c4]=dm_buf[dm_x+0x444]=dm_buf[dm_x+0x5c4]=dm_rd[2])|0x4000;
						dm_buf[dm_x+0x2da]=dm_buf[dm_x+0x45a]=dm_buf[dm_x+0x5da]=
						(dm_buf[dm_x+0x2c5]=dm_buf[dm_x+0x445]=dm_buf[dm_x+0x5c5]=dm_rd[3])|0x4000;
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
					if(ch<16) ed->chestloc[ch++]=map-ed->buf-3;
				case 50: case 103: case 104: case 121:
					drawXx3(4);
					break;
				case 51: case 72:
					drawXx4(4);
					break;
				case 52: case 53: case 56: case 57:
					draw3x2();
					break;
				case 54: case 55: case 77: case 93:
					drawXx3(6);
					break;
				case 58: case 59:
					drawXx3(4);
					dm_wr+=188;
					drawXx3(4);
					break;
				case 78:
					drawXx3(4);
					break;
				case 60: case 61: case 92:
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
				case 75: case 118: case 119:
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
						dm_wr[0xc1]=dm_wr[0xc3]=dm_wr[0xcb]=dm_wr[0xcd]=
						(dm_wr[0xc0]=dm_wr[0xc2]=dm_wr[0xca]=dm_wr[0xcc]=dm_rd[2])|0x4000;
						dm_wr[0xc5]=dm_wr[0xc7]=dm_wr[0xc9]=
						(dm_wr[0xc4]=dm_wr[0xc6]=dm_wr[0xc8]=dm_rd[5])|0x4000;
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
				case 85: case 91:
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
				case 90:
					m=2;
					while(m--) {
						dm_wr[0]=dm_rd[0];
						dm_wr[1]=dm_rd[1];
						dm_wr[2]=dm_rd[2];
						dm_wr[3]=dm_rd[3];
						dm_rd+=4;
						dm_wr+=64;
					}
					break;
				case 96: case 97:
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
				case 105: case 106: case 110: case 111:
					drawXx4(3);
					break;
				case 109: case 108:
					drawXx3(4);
					break;
				case 115:
					fill4x2(rom,nbuf,dm_rd+0x70);
					break;
				case 123:
					tmp=dm_wr;
					draw4x4X(5);
					tmp+=256;
					dm_wr=tmp;
					draw4x4X(5);
					break;
				case 116:
					drawXx4(4);
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
			case 0:
				len1();
				while(dm_l--) draw2x2();
				break;
			case 1: case 2:
				len2();
				while(dm_l--) drawXx4(2),dm_rd=dm_src;
				break;
			case 3:	case 4:
				dm_l++;
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
			case 5: case 6:
				dm_l++;
				while(dm_l--) drawXx4(2),dm_rd=dm_src,dm_wr+=4;
				break;
			case 7: case 8:
				dm_l++;
				while(dm_l--) draw2x2();
				break;
			case 9: case 12: case 13: case 16: case 17: case 20:
				dm_l+=6;
				while(dm_l--) draw1x5(),dm_wr-=63;
				break;
			case 21: case 24: case 25: case 28: case 29: case 32:
				n=-63;
case25:
				dm_l+=6;
				while(dm_l--) {
					dm_buf[dm_x+0x1000]=dm_buf[dm_x]=dm_rd[0];
					dm_buf[dm_x+0x1040]=dm_buf[dm_x+0x40]=dm_rd[1];
					dm_buf[dm_x+0x1080]=dm_buf[dm_x+0x80]=dm_rd[2];
					dm_buf[dm_x+0x10c0]=dm_buf[dm_x+0xc0]=dm_rd[3];
					dm_buf[dm_x+0x1100]=dm_buf[dm_x+0x100]=dm_rd[4];
					dm_x+=n;
				}
				break;
			case 23: case 26: case 27: case 30: case 31:
				n=65;
				goto case25;
			case 10: case 11: case 14: case 15: case 18: case 19: case 22:
				dm_l+=6;
				while(dm_l--) draw1x5(),dm_wr+=65;
				break;
			case 33:
				dm_l=dm_l*2+1;
				drawXx3(2);
				while(dm_l--) dm_rd-=3,drawXx3(1);
				drawXx3(1);
				break;
			case 34:
				dm_l+=2;
case34b:
				if(*dm_wr!=0xe2) *dm_wr=*dm_rd;
case34:
				dm_wr++;
				dm_rd++;
				while(dm_l--) *(dm_wr++)=*dm_rd;
				dm_rd++;
				*dm_wr=*dm_rd;
				break;
			case 35: case 36: case 37: case 38: case 39: case 40: case 41:
			case 42: case 43: case 44: case 45: case 46: case 179: case 180:
				dm_l++;
				n=(*dm_wr)&0x3ff;
				if(!(n==0x1db || n==0x1a6 || n==0x1dd || n==0x1fc))
					*dm_wr=*dm_rd;
				goto case34;
			case 47:
				dm_l+=10;
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
			case 48:
				dm_l+=10;
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
			case 51:
				dm_l++;
				while(dm_l--) dm_rd=dm_src,drawXx4(4);
				break;
			case 52:
				dm_l+=4;
				n=*dm_rd;
				while(dm_l--) *(dm_wr++)=n;
				break;
			case 53:
				break;
			case 54: case 55:
				dm_l++;
				while(dm_l--) dm_rd=dm_src,drawXx4(4),dm_wr+=2;
				break;
			case 56:
				dm_l++;
				while(dm_l--) dm_rd=dm_src,drawXx3(2),dm_wr+=2;
				break;
			case 61:
				dm_l++;
				while(dm_l--) dm_rd=dm_src,drawXx4(2),dm_wr+=4;
				break;
			case 57:
				dm_l++;
				while(dm_l--) dm_rd=dm_src,drawXx4(2),dm_wr+=4;
				break;
			case 58: case 59:
				dm_l++;
				while(dm_l--) dm_rd=dm_src,drawXx3(4),dm_wr+=4;
				break;
			case 60:
				dm_l++;
				while(dm_l--) {
					dm_rd=dm_src,draw2x2();
					dm_wr+=0x17e;
					dm_rd+=4;
					draw2x2();
					dm_wr-=0x17e;
				}
				break;
			case 62: case 75:
				dm_l++;
				while(dm_l--) draw2x2(),dm_wr+=12;
				break;
			case 63: case 64: case 65: case 66: case 67: case 68: case 69:
			case 70:
				dm_l++;
				n=(*dm_wr)&0x3ff;
				if(!(n==0x1db || n==0x1a6 || n==0x1dd || n==0x1fc)) *dm_wr=*dm_rd;
				dm_rd++;
				dm_wr++;
				n=*(dm_rd++);
				while(dm_l--) *(dm_wr++)=n;
				*dm_wr=*dm_rd;
				break;
			case 71:
				dm_l++;
				dm_l<<=1;
				draw1x5();
				dm_rd+=5;
				dm_wr++;
				while(dm_l--) draw1x5(),dm_wr++;
				dm_rd+=5;
				draw1x5();
				break;
			case 72:
				dm_l++;
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
			case 73: case 74:
				dm_l++;
				draw4x2(4);
				break;
			case 76:
				dm_l++;
				dm_l<<=1;
				drawXx3(1);
				while(dm_l--) drawXx3(1),dm_rd-=3;
				dm_rd+=3;
				drawXx3(1);
				break;
			case 77: case 78: case 79:
				dm_l++;
				drawXx4(1);
				while(dm_l--) drawXx4(2),dm_rd-=8;
				dm_rd+=8;
				drawXx4(1);
				break;
			case 80:
				dm_l+=2;
				n=*dm_rd;
				while(dm_l--) *(dm_wr++)=n;
				break;
			case 81: case 82: case 91: case 92:
				drawXx3(2);
				while(dm_l--) drawXx3(2),dm_rd-=6;
				dm_rd+=6;
				drawXx3(2);
				break;
			case 83:
				dm_l++;
				while(dm_l--) draw2x2();
				break;
			case 85: case 86:
				dm_l++;
				draw4x2(12);
				break;
			case 93:
				dm_l+=2;
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
			case 94:
				dm_l++;
				while(dm_l--) draw2x2(),dm_wr+=2;
				break;
			case 95:
				dm_l+=21;
				goto case34b;
			case 96: case 146: case 147:
				len1();
				while(dm_l--) draw2x2(),dm_wr+=126;
				break;
			case 97: case 98:
				len2();
				draw4x2(0x80);
				break;
			case 99: case 100:
				dm_l++;
case99:
				while(dm_l--) {
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
			case 101: case 102:
				dm_l++;
				draw4x2(0x180);
				break;
			case 103: case 104:
				dm_l++;
				while(dm_l--) draw2x2(),dm_wr+=126;
				break;
			case 105:
				dm_l+=2;
				if((*dm_wr&0x3ff)!=0xe3) *dm_wr=*dm_rd;
				dm_wr+=64;
				while(dm_l--) *dm_wr=dm_rd[1],dm_wr+=64;
				*dm_wr=dm_rd[2];
				break;
			case 106: case 107: case 121: case 122:
				dm_l++;
				while(dm_l--) *dm_wr=*dm_rd,dm_wr+=64;
				break;
			case 108:
				dm_l+=10;
				n=*(dm_rd++);
				if((*dm_wr&0x3ff)!=0xe3) draw9078(n);
				dm_rd+=2;
				dm_wr+=128;
				while(dm_l--) *dm_wr=*dm_rd,dm_wr[1]=n,dm_wr+=64;
				dm_rd++;
				draw9078(n);
				break;
			case 109:
				dm_l+=10;
				n=*(dm_rd++);
				if((dm_wr[1]&0x3ff)!=0xe3) draw90c2(n);
				dm_rd+=2;
				dm_wr+=128;
				while(dm_l--) *dm_wr=n,dm_wr[1]=*dm_rd,dm_wr+=64;
				dm_rd++;
				draw90c2(n);
				break;
			case 112:
				dm_l++;
				while(dm_l--) dm_rd=dm_src,drawXx4(4),dm_wr+=252;
				break;
			case 113:
				dm_l+=4;
				while(dm_l--) *dm_wr=*dm_rd,dm_wr+=64;
				break;
			case 115: case 116:
				dm_l++;
				while(dm_l--) dm_rd=dm_src,drawXx4(4),dm_wr+=0x17c;
				break;
			case 117:
				dm_l++;
				while(dm_l--) dm_rd=dm_src,drawXx4(2),dm_wr+=0x17e;
				break;
			case 118: case 119:
				dm_l++;
				while(dm_l--) dm_rd=dm_src,drawXx4(3),dm_wr+=0x1fd;
				break;
			case 120: case 123:
				dm_l++;
				while(dm_l--) dm_rd=dm_src,draw2x2(),dm_wr+=0x37e;
				break;
			case 124:
				dm_l+=2;
				while(dm_l--) *dm_wr=*dm_rd,dm_wr+=64;
				break;
			case 125:
				dm_l++;
				while(dm_l--) dm_rd=dm_src,draw2x2(),dm_wr+=0x7e;
				break;
			case 127: case 128:
				dm_l++;
				while(dm_l--) dm_rd=dm_src,drawXx4(2),dm_wr+=0x2fe;
				break;
			case 129: case 130: case 131: case 132:
				dm_l++;
				while(dm_l--) dm_rd=dm_src,drawXx4(3),dm_wr+=0x1fd;
				break;
			case 133: case 134:
				draw3x2(),dm_wr+=128,dm_rd+=6;
				while(dm_l--) draw3x2(),dm_wr+=128;
				dm_rd+=6;
				draw3x2();
				break;
			case 135:
				dm_l++;
				while(dm_l--) dm_rd=dm_src,drawXx4(2),dm_wr+=0x17e;
				break;
			case 136:
				dm_l++;
				draw2x2(),dm_wr+=0x7e;
				dm_rd+=4;
				while(dm_l--) dm_wr[0]=dm_rd[0],dm_wr[1]=dm_rd[1],dm_wr+=64;
				dm_rd+=2;
				drawXx3(2);
				break;
			case 137:
				dm_l++;
				while(dm_l--) dm_rd=dm_src,draw2x2(),dm_wr+=0xfe;
				break;
			case 138:
				dm_l+=21;
				if((*dm_wr&0x3ff)!=0xe3) *dm_wr=*dm_rd;
				dm_wr+=64;
				while(dm_l--) *dm_wr=dm_rd[1],dm_wr+=64;
				*dm_wr=dm_rd[2];
				break;
			case 140: case 139:
				dm_l+=8;
				while(dm_l--) *dm_wr=*dm_rd,dm_wr+=64;
				break;
			case 141: case 142:
				dm_l++;
				while(dm_l--) *dm_wr=*dm_rd,dm_wr+=64;
				break;
			case 143:
				dm_l+=2;
				dm_l<<=1;
				dm_wr[0]=dm_rd[0];
				dm_wr[1]=dm_rd[1];
				while(dm_l--) dm_wr[64]=dm_rd[2],dm_wr[65]=dm_rd[3],dm_wr+=64;
				break;
			case 144: case 145:
				len2();
				draw4x2(0x80);
				break;
			case 148:
				dm_l++;
				while(dm_l--) dm_rd=dm_src,drawXx4(4),dm_wr+=0xfc;
				break;
			case 149: case 150:
				dm_l++;
				while(dm_l--) draw2x2(),dm_wr+=0x7e;
				break;
			case 160: case 169:
				dm_l+=4;
				for(;dm_l;dm_l--) {
					n=dm_l;
					drawtr();
					dm_wr+=64;
				}
				break;
			case 161: case 166: case 170:
				dm_l+=4;
				m=0;
				while(dm_l--) {
					m++;
					n=m;
					for(l=0;l<n;l++) dm_wr[l]=*dm_rd;
					dm_wr+=64;
				}
				break;
			case 162: case 167: case 171:
				dm_l+=4;
				for(;dm_l;dm_l--) {
					drawtr();
					dm_wr+=65;
				}
				break;
			case 163: case 168: case 172:
				dm_l+=4;
				for(;dm_l;dm_l--) {
					drawtr();
					dm_wr-=63;
				}
				break;
			case 164:
				dm_l+=4;
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
			case 165:
				dm_l+=4;
				for(;dm_l;dm_l--) drawtr(),dm_wr+=64;
				break;
			case 176: case 177:
				dm_l+=8;
				drawtr();
				break;
			case 178:
				dm_l++;
				while(dm_l--) dm_rd=dm_src,drawXx4(4);
				break;
			case 181:
				dm_l++; goto c182;
			case 182: case 183:
				len2();
c182:
				while(dm_l--) drawXx4(2),dm_rd-=8;
				break;
			case 184: // yeah!
				len2();
				while(dm_l--) draw2x2();
				break;
			case 185:
				len1();
				while(dm_l--) draw2x2();
				break;
			case 186:
				dm_l++;
				while(dm_l--) dm_rd=dm_src,drawXx4(4);
				break;
			case 187:
				dm_l++;
				while(dm_l--) draw2x2(),dm_wr+=2;
				break;
			case 188: case 189:
				dm_l++;
				while(dm_l--) draw2x2();
				break;
			case 192: case 194:
				l=(dm_l&3)+1;
				dm_l>>=2;
				dm_l++;
				while(l--) {
					m=dm_l;
					dm_src=dm_wr;
					while(m--)
						dm_wr[0]=dm_wr[1]=dm_wr[2]=dm_wr[3]=
						dm_wr[64]=dm_wr[65]=dm_wr[66]=dm_wr[67]=
						dm_wr[128]=dm_wr[129]=dm_wr[130]=dm_wr[131]=
						dm_wr[192]=dm_wr[193]=dm_wr[194]=dm_wr[195]=*dm_rd,dm_wr+=4;
					dm_wr=dm_src+256;
				}
				break;
			case 193:
				l=(dm_l&3)+1;
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
			case 195: case 215:
				l=(dm_l&3)+1;
				dm_l>>=2;
				dm_l++;
				while(l--) {
					tmp=dm_wr;
					m=dm_l;
					while(m--) {
						dm_wr[0]=dm_wr[1]=dm_wr[2]=dm_wr[64]=dm_wr[65]=dm_wr[66]=
						dm_wr[128]=dm_wr[129]=dm_wr[130]=*dm_rd;
						dm_wr+=3;
					}
					dm_wr=tmp+0xc0;
				}
				break;
			case 196:
				dm_rd+=(ed->buf[0]&15)<<3;
			case 200: case 197: case 198: case 199: case 201: case 202:
				l=(dm_l&3);
				dm_l>>=2;
				dm_l++;
				l++;
				goto grid4x4;
			case 205:
				l=dm_l&3;
				dm_l>>=2;
				m=(unsigned char)(((short*)(rom+0x1b0a))[dm_l]);
				n=((short*)(rom+0x1b12))[l];
				dm_src=dm_wr;
				dm_wr-=n;
				tmp=dm_wr;
				dm_rd=(short*)(rom+0x1f2a);
				while(n--) {
					dm_wr[0]=dm_rd[0];
					o=(m<<1)+4;
					while(o--) {
						dm_wr[64]=dm_rd[1];
						dm_wr+=64;
					}
					dm_wr[64]=dm_rd[2];
					tmp++;
					dm_wr=tmp;
				}
				dm_wr=dm_src;
				dm_x=dm_src-nbuf-1;
				o=dm_x&31;
				if(dm_x&32) o|=0x200;
				o|=0x800;
				dm_wr=dm_src;
				dm_rd=(short*)(rom+0x227c);
				drawXx3(3);
				dm_wr+=0xbd;
				while(m--) draw3x2(),dm_wr+=128;
				dm_rd+=6;
				drawXx3(3);
				break;
			case 206:
				dm_rd=(short*)(rom+0x22ac);
				tmp=dm_wr;
				drawXx3(3);
				l=dm_l&3;
				dm_l>>=2;
				o=m=(unsigned char)(((short*)(rom+0x1b0a))[dm_l]);
				n=((short*)(rom+0x1b12))[l];
				dm_wr=tmp+0xc0;
				while(o--) draw3x2(),dm_wr+=128;
				dm_rd+=6;
				drawXx3(3);
				tmp+=3;
				dm_rd=(short*)(rom+0x1f2a);
				m<<=1;
				m+=4;
				while(n--) {
					dm_wr=tmp;
					dm_wr[0]=dm_rd[0];
					o=m;
					while(o--) {
						dm_wr[64]=dm_rd[1];
						dm_wr+=64;
					}
					dm_wr[64]=dm_rd[2];
					tmp++;
				}
				break;
			case 209: case 210: case 217: case 223: case 224: case 225:
			case 226: case 227: case 228: case 229: case 230: case 231:
			case 232:
				l=(dm_l&3);
				dm_l>>=2;
				dm_l++;
				l++;
grid4x4:
				while(l--) {
					tmp=dm_wr;
					draw4x4X(dm_l);
					dm_wr=tmp+0x100;
				}
				break;
			case 216: case 218:
				l=dm_l&3;
				dm_l>>=2;
				dm_l=(unsigned char)(((unsigned short*)(rom+0x1b3a))[dm_l]);
				l=(unsigned char)(((unsigned short*)(rom+0x1b3a))[l]);
				dm_rd=(unsigned short*)(rom+0x1c62);
				goto grid4x4;
				break;
			case 219:
				l=(dm_l&3)+1;
				dm_l>>=2;
				dm_l++;
				dm_rd+=(ed->buf[0]&240)>>1;
				goto grid4x4;
				break;
			case 220:
				l=((dm_l&3)<<1)+5;
				dm_l=(dm_l>>2)+1;
				dm_tmp=dm_wr;
				while(l--) {
					draw975c();
				}
				dm_rd++;
				draw975c();
				dm_rd++;
				draw975c();
				break;
			case 221:
				l=((dm_l&3)<<1)+1;
				dm_l=(dm_l>>2)+1;
				tmp=dm_wr;
				draw93ff();
				dm_rd+=4;
				while(l--) draw93ff();
				dm_rd+=4;
				draw93ff();
				dm_rd+=4;
				draw93ff();
				break;
			case 222: // Yup. It's 222!
				l=(dm_l&3)+1;
				dm_l=(dm_l>>2)+1;
				while(l--) {
					m=dm_l;
					tmp=dm_wr;
					while(m--) draw2x2();
					tmp+=128;
					dm_wr=tmp;
				}
				break;
			}
		}
	}
end:
	ed->chestnum=ch;
	return map+2;
}

void Updatemap(DUNGEDIT*ed)
{
	int i;
	short*nbuf=ed->nbuf,*buf2;
	unsigned char*buf=ed->buf;
	unsigned char*rom=ed->ew.doc->rom;
	dm_buf=nbuf;
	buf2=(short*)(rom+0x1b52+((buf[0]<<4)&0xf0));
	fill4x2(rom,ed->nbuf,buf2);
	buf2=(short*)(rom+0x1b52+(buf[0]&0xf0));
	fill4x2(rom,ed->nbuf+0x1000,buf2);
	ed->chestnum=0;
	if(ed->ew.param<0x8c) {
		Drawmap(rom,ed->nbuf,rom+romaddr(*(int*)(rom+romaddr(*(int*)(rom+0x882d))+(buf[1]>>2)*3)),ed);
		buf=Drawmap(rom,ed->nbuf,buf+2,ed);
		buf=Drawmap(rom,ed->nbuf+0x1000,buf,ed);
		Drawmap(rom,ed->nbuf,buf,ed);
		for(i=0;i<0x18c;i+=4) {
			if(*(short*)(rom+0x271de+i)==ed->mapnum) {
				dm_x=(*(short*)(rom+0x271e0+i)&0x3fff)>>1;
				ed->nbuf[dm_x]=0x1922;
				ed->nbuf[dm_x+64]=0x1932;
				ed->nbuf[dm_x+1]=0x1923;
				ed->nbuf[dm_x+65]=0x1933;
			}
		}
		for(i=2;i<ed->tsize;i+=2) {
			dm_x=*(short*)(ed->tbuf+i)&0x1fff;
			dm_x>>=1;
			ed->nbuf[dm_x]=0xde0;
			ed->nbuf[dm_x+64]=0xdf0;
			ed->nbuf[dm_x+1]=0x4de0;
			ed->nbuf[dm_x+65]=0x4df0;
		}
	} else Drawmap(rom,ed->nbuf,buf+2,ed);
}
const static char *mus_str[]={
	"Same",
	"None",
	"Title",
	"World map",
	"Beginning",
	"Rabbit",
	"Forest",
	"Intro",
	"Town",
	"Warp",
	"Dark world",
	"Master swd",
	"File select",
	"Soldier",
	"Mountain",
	"Shop",
	"Fanfare",
	"Castle",
	"Palace",
	"Cave",
	"Clear",
	"Church",
	"Boss",
	"Dungeon",
	"Psychic",
	"Secret way",
	"Rescue",
	"Crystal",
	"Fountain",
	"Pyramid",
	"Kill Agah",
	"Ganon room",
	"Last boss",
	"Triforce",
	"Ending",
	"Staff",
	"Stop",
	"Fade out",
	"Lower vol",
	"Normal vol"
};
static int sbank_ofs[]={0xc8000,0,0xd8000,0};
char op_len[32]=
	{1,1,2,3,0,1,2,1,2,1,1,3,0,1,2,3,1,3,3,0,1,3,0,3,3,3,1,2,0,0,0,0};
short spcbank;
unsigned short spclen;
unsigned char*Getspcaddr(FDOC*doc,unsigned short addr,short bank)
{
	unsigned char*rom;
	unsigned short a,b;
	spcbank=bank+1;
again:
	rom=doc->rom+sbank_ofs[spcbank];
	for(;;) {
		a=*(unsigned short*)rom;
		if(!a) if(spcbank) { spcbank=0;goto again; } else return 0;
		b=*(unsigned short*)(rom+2);
		rom+=4;
		if(addr>=b && addr-b<a) {spclen=a; return rom+addr-b;}
		rom+=a;
	}
}
short lastsr;
short ss_lasttime;
short Getblocktime(FDOC*doc,short num,short prevtime)
{
	SCMD*sc=doc->scmd,*sc2;
	int i=-1,j=0,k=0,l,m=0,n=prevtime;
	l=num;
	if(l==-1) return 0;
	for(;;) {
		if(sc[l].flag&4) {
			j=sc[l].tim; m=sc[l].tim2; k=1;
		}
		if(!k) i=l;
		if(sc[l].flag&1) n=sc[l].b1;
		l=sc[l].next;
		if(l==-1) {if(!k) m=0,j=0;break;}
	}
	if(i!=-1) for(;;) {
		if(i==-1) {
			MessageBox(framewnd,"Really bad error","Bad error happened",MB_OK);
			doc->m_modf=1;
			return 0;
		}
		sc2=sc+i;
		if(sc2->cmd==0xef) {
			k=*(short*)&(sc2->p1);
			if(k>=doc->m_size) {MessageBox(framewnd,"Invalid music address","Bad error happened",MB_OK); doc->m_modf=1; return 0;}
			if(sc2->flag&1) {
				j+=Getblocktime(doc,k,0)*sc2->p3;
				if(ss_lasttime) {
					j+=ss_lasttime*m;
					j+=sc[k].tim2*sc2->b1;
					j+=(sc2->p3-1)*sc[k].tim2*ss_lasttime;
				} else {
					j+=sc2->b1*(m+sc[k].tim2*sc2->p3);
				}
				m=0;
			} else {
				j+=Getblocktime(doc,k,0)*sc2->p3;
				j+=ss_lasttime*m;
				if(ss_lasttime) j+=(sc2->p3-1)*ss_lasttime*sc[k].tim2,m=sc[k].tim2;
				else m+=sc[k].tim2*sc2->p3;
			}
		} else {
			if(sc2->cmd<0xe0) m++;
			if(sc2->flag&1) {j+=m*sc[i].b1; m=0; }
		}
		sc2->tim=j;
		sc2->tim2=m;
		sc2->flag|=4;
		if(i==num) break;
		i=sc2->prev;
	}
	ss_lasttime=n;
	return sc[num].tim+prevtime*sc[num].tim2;
}
short Loadscmd(FDOC*doc,unsigned short addr,short bank,int t)
{
	int b,c,d,e,f,g,h,i,l,m,n,o;
	unsigned char j,k;
	unsigned char*a;
	SRANGE*sr;
	SCMD*sc=doc->scmd,*sc2;
	if(!addr) return -1;
	a=Getspcaddr(doc,addr,bank);
	d=spcbank;
	if(!a) {
		MessageBox(framewnd,"Address not found when loading track","Bad error happened",MB_OK);
		return -1;
	}
	sr=doc->sr;
	e=doc->srnum;
	f=0x10000;
	for(c=0;c<e;c++) {
		if(sr[c].bank==d) if(sr[c].start>addr) {if(sr[c].start<f) f=sr[c].start; n=c;}
		else if(sr[c].end>addr) {
			for(f=sr[c].first;f!=-1;f=sc[f].next) {
				if(sc[f].flag&4) m=sc[f].tim,o=sc[f].tim2;
				if(sc[f].addr==addr) {
					sc[f].tim=m;
					sc[f].tim2=o;
					lastsr=c;
					return f;
				}
				if(sc[f].flag&1) k=sc[f].b1;
				if(sc[f].cmd<0xca) if(k) m-=k; else o--;
			}
			MessageBox(framewnd,"Misaligned music pointer","Bad error happened",MB_OK);
			return -1;
		}
	}
	c=n;
	i=h=doc->m_free;
	a-=addr;
	m=0;
	k=0;
	o=0;
	for(g=addr;g<f;) {
		sc2=sc+i;
		if(sc2->next==-1) {
			l=doc->m_size;
			sc=doc->scmd=realloc(sc,sizeof(SCMD)*(doc->m_size+=1024));
			sc2=sc+i;
			n=l+1023;
			while(l<n) sc[l].next=l+1,l++;
			sc[l].next=-1;
			n-=1023;
			while(l>n) sc[l].prev=l-1,l--;
			sc[l].prev=i;
			sc2->next=l;
		}
		sc2->addr=g;
		b=a[g];
		if(!b) break;
		if(m>=t && b!=0xf9) break;
		g++;
		j=0;
		if(b<128) {
			j=1;
			k=sc2->b1=b;
			b=a[g++];
			if(b<128) j=3,sc2->b2=b,b=a[g++];
		}
		if(b<0xe0) if(k) m+=k; else o++;
		sc2->cmd=b;
		sc2->flag=j;
		if(b>=0xe0) {
			b-=0xe0;
			if(op_len[b]) sc2->p1=a[g++];
			if(op_len[b]>1) sc2->p2=a[g++];
			if(op_len[b]>2) sc2->p3=a[g++];
			if(b==15) {
				doc->m_free=sc2->next;
				sc[sc2->next].prev=-1;
				l=Loadscmd(doc,*(short*)(&(sc2->p1)),bank,t-m);
				sc=doc->scmd;
				sc2=sc+i;
				*(short*)(&(sc2->p1))=l;
				sc2->next=doc->m_free;
				sc[sc2->next].prev=i;
				Getblocktime(doc,l,0);
				if(sc[l].flag&4) m+=(sc[l].tim+sc[l].tim2*k)*sc2->p3; else {i=sc2->next;break;}
				if(doc->sr[lastsr].endtime) k=doc->sr[lastsr].endtime;
			}
		}
		i=sc2->next;
	}
	sc[h].tim=m;sc[h].tim2=o;sc[h].flag|=4;
	if(f==g && m<t) {
		l=sc[i].prev;
		lastsr=c;
		sc[sr[lastsr].first].prev=l;
		l=sc[l].next=sr[lastsr].first;
		if(sc[l].flag&4) sc[h].tim=sc[l].tim+m+sc[l].tim2*k,sc[h].flag|=4;
		sr[lastsr].first=h;
		sr[lastsr].start=addr;
		sr[lastsr].inst++;
	} else {
		if(doc->srsize==doc->srnum) doc->sr=realloc(doc->sr,(doc->srsize+=16)*sizeof(SRANGE));
		lastsr=doc->srnum;
		sr=doc->sr+(doc->srnum++);
		sr->start=addr;
		sr->end=g;
		sr->first=h;
		sr->endtime=k;
		sr->inst=1;
		sr->editor=0;
		sr->bank=d;
		sc[sc[i].prev].next=-1;
	}
	sc[i].prev=-1;
	doc->m_free=i;
	return h;
}
char fil1[4]={0,15,61,115};
char fil2[4]={0,4,5,6};
char fil3[4]={0,0,15,13};

int ss_num,ss_size;
unsigned short ss_next=0;
SSBLOCK* Allocspcblock(int len,int bank)
{
	SSBLOCK*sbl;
	if(!len) {
		MessageBox(framewnd,"Warning zero length block allocated","Bad error happened",MB_OK);
	}
	if(ss_num==ss_size) {
		ss_size+=512;
		ssblt=realloc(ssblt,ss_size<<2);
	}
	ssblt[ss_num]=sbl=malloc(sizeof(SSBLOCK));
	ss_num++;
	sbl->start=ss_next;
	sbl->len=len;
	sbl->buf=malloc(len);
	sbl->relocs=malloc(32);
	sbl->relsz=16;
	sbl->relnum=0;
	sbl->bank=bank&7;
	sbl->flag=bank>>3;
	ss_next+=len;
	return sbl;
}
void Addspcreloc(SSBLOCK*sbl,short addr)
{
	sbl->relocs[sbl->relnum++]=addr;
	if(sbl->relnum==sbl->relsz) {
		sbl->relsz+=16;
		sbl->relocs=realloc(sbl->relocs,sbl->relsz<<1);
	}
}
short Savescmd(FDOC*doc,short num,short songtime,short endtr)
{
	SCMD*sc=doc->scmd,*sc2;
	SRANGE*sr=doc->sr;
	SSBLOCK*sbl;
	unsigned char*b;
	int i=num,j,k=0,l,m,n=0,o=0,p;
	if(i==-1) return 0;
	if(i>=doc->m_size) {MessageBox(framewnd,"Error.","Bad error happened",MB_OK); doc->m_modf=1; return 0;}
	if(sc[i].flag&8) return sc[i].addr;
	for(;;) {
		j=sc[i].prev;
		if(j==-1) break;
		i=j;
	}
	for(j=0;j<doc->srnum;j++) {
		if(sr[j].first==i) {
			l=Getblocktime(doc,i,0);
			m=i;
			for(;;) {
				if(m==-1) break;
				k++;
				sc2=sc+m;
				if(sc2->flag&1) k++,n=sc2->b1;
				if(sc2->flag&2) k++;
				if(sc2->cmd>=0xe0) k+=op_len[sc2->cmd-0xe0];
				m=sc2->next;
			}
			songtime-=l;
			if(songtime>0) {
				l=(songtime+126)/127;
				if(songtime%l) l+=2;
				l++;
				if(n && !songtime%n) {
					p=songtime/n;
					if(p<l) l=p;
				} else p=-1;
				k+=l;
			}
			k++;
			sbl=Allocspcblock(k,sr[j].bank|((!endtr)<<3)|16);
			b=sbl->buf;
			for(;;) {
				if(i==-1) break;
				sc2=sc+i;
				sc2->addr=b-sbl->buf+sbl->start;
				sc2->flag|=8;
				if(sc2->flag&1) *(b++)=sc2->b1;
				if(sc2->flag&2) *(b++)=sc2->b2;
				*(b++)=sc2->cmd;
				if(sc2->cmd>=0xe0) {
					o=op_len[sc2->cmd-0xe0];
					if(sc2->cmd==0xef) {
						*(short*)b=Savescmd(doc,*(short*)&(sc2->p1),0,1);
						if(b) Addspcreloc(sbl,b-sbl->buf);
						b[2]=sc2->p3;
						b+=3;
					} else {
						if(o) *(b++)=sc2->p1;
						if(o>1) *(b++)=sc2->p2;
						if(o>2) *(b++)=sc2->p3;
					}
				}
				i=sc2->next;
			}
			if(songtime>0) {
				if(l!=p) {
					l=(songtime+126)/127;
					if(songtime%l) n=127; else n=songtime/l;
					*(b++)=n;
				}
				for(;songtime>=n;songtime-=n) *(b++)=0xc9;
				if(songtime) {*(b++)=songtime;*(b++)=0xc9;}
			}
			*(b++)=0;
			return sc[num].addr;
		}
	}
	wsprintf(buffer,"Address %04X not found",num);
	MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
	doc->m_modf=1;
	return 0;
}
int Writespcdata(FDOC*doc,void*buf,int len,int addr,int spc,int limit)
{
	unsigned char*rom=doc->rom;
	if(!len) return addr;
	if(((addr+len+4)&0x7fff)>0x7ffb) {
		if(addr+5>limit) goto error;
		*(int*)(rom+addr)=0x00010140;
		rom[addr+4]=0xff;
		addr+=5;
	}
	if(addr+len+4>limit) {
error:
		MessageBox(framewnd,"Not enough space for sound data","Bad error happened",MB_OK);
		doc->m_modf=1;
		return 0xc8000;
	}
	*(short*)(rom+addr)=len;
	*(short*)(rom+addr+2)=spc;
	memcpy(rom+addr+4,buf,len);
	return addr+len+4;
}
void Modifywaves(FDOC*doc,int es)
{
	int i,j;
	ZWAVE*zw=doc->waves,*zw2=zw+es;
	j=doc->numwave;
	for(i=0;i<j;i++) {
		if(zw->copy==es) {
			if(zw->end!=zw2->end) {
				zw->end=zw2->end;
				zw->buf=realloc(zw->buf,zw->end<<1);
			}
			memcpy(zw->buf,zw2->buf,zw->end<<1);
			if(zw->lopst>=zw->end) zw->lflag=0;
			if(zw->lflag) zw->buf[zw->end]=zw->buf[zw->lopst];
		}
		zw++;
	}
}

void Savesongs(FDOC *doc)
{
	int i,j,k,l=0,m,n,o,p,q,r,t,u,v,w,a,e,f,g;
	unsigned short bank_next[4],bank_lwr[4];
	short *c, *d;
	unsigned char *rom, *b;
	
	SONG *s;
	SCMD *sc;
	SONGPART *sp;
	SSBLOCK *stbl, *sptbl, *trtbl, *pstbl;
	ZWAVE *zw, *zw2;
	ZINST *zi;
	SAMPEDIT *sed;
	
	short wtbl[128],x[16],y[18];
	unsigned char z[64];
	
	ss_num = 0;
	ss_size = 512;
	ss_next = 0;
	ssblt = malloc(512 * sizeof(SSBLOCK));
	
	if(! (doc->m_modf) ) // if the music has not been modified, return. 
		return;
	
	doc->m_modf = 0; // set it so the music has not been modified. (reset the status)
	rom = doc->rom;
	
	SetCursor(wait_cursor);
	
	for(i = 0; i<3; i++)
	{
		k = doc->numsong[i];
	
		for(j=0;j<k;j++) 
		{
			s = doc->songs[l++];
			
			if(!s) 
				continue;
			
			s->flag &= -5;
			
			for(m=0;m<s->numparts;m++) 
			{
				sp = s->tbl[m];
				sp->flag &= -3;
			}
		}
	}
	
	j = doc->m_size;
	sc = doc->scmd;
	
	for(i = 0; i<j; i++) 
	{
		sc->flag &= -13;
		sc++;
	}
	
	l = 0;
	
	for(i=0;i<3;i++) 
	{
		k = doc->numsong[i];
		stbl = Allocspcblock(k<<1,i+1);
		
		for(j = 0; j < k; j++) 
		{
			s = doc->songs[l++];
			
			if(!s) 
			{
				( (short*) (stbl->buf) )[j] = 0;
				
				continue;
			}
			
			if(s->flag&4) 
				goto alreadysaved;
			
			sptbl = Allocspcblock(((s->numparts+1)<<1)+(s->flag&2),(s->flag&1)?0:(i+1));
			
			for(m = 0; m < s->numparts; m++) 
			{
				sp = s->tbl[m];
				
				if(sp->flag&2) 
					goto spsaved;
				
				trtbl = Allocspcblock(16,(sp->flag&1)?0:(i+1));
				
				p = 0;
				
				for(n = 0; n < 8; n++) 
				{
					o = Getblocktime(doc,sp->tbl[n],0);
					
					if(o > p) 
						p = o;
				}
				
				q = 1;
				
				for(n = 0; n < 8; n++) 
				{
					if(((short*)(trtbl->buf))[n] = Savescmd(doc,sp->tbl[n],p,q))
						Addspcreloc(trtbl,n<<1),q=0;
				}

				sp->addr = trtbl->start;
				sp->flag |= 2;
spsaved:
				( (short*) (sptbl->buf) )[m] = sp->addr;
				
				Addspcreloc(sptbl,m<<1);
			}
			
			if(s->flag&2) 
			{
				( (short*) (sptbl->buf) )[m++] = 255;
				( (short*) (sptbl->buf) )[m] = sptbl->start + (s->lopst << 1);
				
				Addspcreloc(sptbl,m<<1);
			} 
			else 
				( (short*) (sptbl->buf) )[m++] = 0;
			
			s->addr = sptbl->start;
			s->flag |= 4;
alreadysaved:
			( (short*) (stbl->buf) )[j] = s->addr;
			
			Addspcreloc(stbl,j << 1);
		}
	}
	
	if(doc->w_modf) 
	{
		b = malloc(0xc000);
		j = 0;
	
		zw = doc->waves;
	
		if(doc->mbanks[3]) 
			sed = (SAMPEDIT*)GetWindowLong(doc->mbanks[3],GWL_USERDATA); 
		else 
			sed = 0;
	
		for(i = 0; i < doc->numwave; i++, zw++) 
		{
			if(zw->copy != -1) 
				continue;
		
			wtbl[i << 1] = j + 0x4000;
		
			if(zw->lflag) 
			{
				l = zw->end-zw->lopst;
			
				if(l&15) 
				{
					k = (l << 15)/((l + 15) & -16);
					p = (zw->end << 15)/k;
					c = malloc(p<<1);
					n = 0;
					d = zw->buf;
				
					for(m = 0;;) 
					{
						c[n++] = (d[m >> 15] * ( (m & 32767)^32767) + d[(m>>15)+1]*(m&32767))/32767;
					m += k;
					
					if(n >= p) 
						break;
				}
			
					zw->lopst = (zw->lopst << 15)/k;
				zw->end = p;
				zw->buf = realloc(zw->buf,zw->end+1<<1);
				memcpy(zw->buf,c,zw->end<<1);
				free(c);
				zw->buf[zw->end]=zw->buf[zw->lopst];
				zw2=doc->waves;
				for(m=0;m<doc->numwave;m++,zw2++) if(zw2->copy==i)
					zw2->lopst=zw2->lopst<<15/k;
				zi=doc->insts;
				for(m=0;m<doc->numinst;m++) {
					n=zi->samp;
					if(n>=doc->numwave) continue;
					if(n==i || doc->waves[n].copy==i) {
						o=(zi->multhi<<8)+zi->multlo;
						o=(o<<15)/k;
						zi->multlo=o;
						zi->multhi=o>>8;
						if(sed && sed->editinst==m) {
							sed->init=1;
							SetDlgItemInt(sed->dlg,3014,o,0);
							sed->init=0;
						}
					}
					zi++;
				}
				Modifywaves(doc,i);
			}
		}
		k=(-zw->end)&15;
		d=zw->buf;
		n=0;
		wtbl[(i<<1)+1]=(zw->lopst+k>>4)*9+wtbl[i<<1];
		y[0]=y[1]=0;
		u=4;
		for(;;) {
			for(o=0;o<16;o++) {
				if(k) k--,x[o]=0;
				else x[o]=d[n++];
			}
			p=0x7fffffff;
			a=0;
			for(t=0;t<4;t++) {
				r=0;
				for(o=0;o<16;o++) {
					l=x[o];
					y[o+2]=l;
					l+=(y[o]*fil3[t]>>4)-(y[o+1]*fil1[t]>>fil2[t]);
					if(l>r) r=l;
					else if(-l>r) r=-l;
				}
				r<<=1;
				if(t) m=14; else m=15;
				for(q=0;q<12;q++,m+=m) if(m>=r) break;
tryagain:
				if(q && (q<12 || m==r)) v=(1<<q-1)-1; else v=0;
				m=0;
				for(o=0;o<16;o++) {
					l=(y[o+1]*fil1[t]>>fil2[t])-(y[o]*fil3[t]>>4);
					w=x[o];
					r=w-l+v>>q;
					if((r+8)&0xfff0) {q++; a-=o; goto tryagain;}
					z[a++]=r;
					l=(r<<q)+l;
					y[o+2]=l;
					l-=w;
					m+=l*l;
				}
				if(u==4) { u=0,e=q,f=y[16],g=y[17]; break; }
				if(m<p) p=m,u=t,e=q,f=y[16],g=y[17];
			}
			m=(e<<4)|(u<<2);
			if(n==zw->end) m|=1;
			if(zw->lflag) m|=2;
			b[j++]=m;
			m=0;
			a=u<<4;
			for(o=0;o<16;o++) {
				m|=z[a++]&15;
				if(o&1) b[j++]=m,m=0; else m<<=4;
			}
			if(n==zw->end) break;
			y[0]=f;
			y[1]=g;
		}
	}
	if(sed) {
		SetDlgItemInt(sed->dlg,3008,sed->zw->end,0);
		SetDlgItemInt(sed->dlg,3010,sed->zw->lopst,0);
		InvalidateRect(GetDlgItem(sed->dlg,3002),0,1);
	}
	zw=doc->waves;
	for(i=0;i<doc->numwave;i++,zw++) if(zw->copy!=-1) {
			wtbl[i<<1]=wtbl[zw->copy<<1];
			wtbl[(i<<1)+1]=(zw->lopst>>4)*9+wtbl[i<<1];
		}
	m=Writespcdata(doc,wtbl,doc->numwave<<2,0xc8000,0x3c00,0xd74fc);
	m=Writespcdata(doc,b,j,m,0x4000,0xd74fc);
	free(b);
	m=Writespcdata(doc,doc->insts,doc->numinst*6,m,0x3d00,0xd74fc);
	m=Writespcdata(doc,doc->snddat1,doc->sndlen1,m,0x800,0xd74fc);
	m=Writespcdata(doc,doc->snddat2,doc->sndlen2,m,0x17c0,0xd74fc);
	m=Writespcdata(doc,doc->sndinsts,doc->numsndinst*9,m,0x3e00,0xd74fc);
	doc->m_ofs=m;
	} else m=doc->m_ofs;
	bank_next[0]=0x2880;
	bank_next[1]=0xd000;
	bank_next[2]=0xd000;
	bank_next[3]=0xd000;
	bank_lwr[0]=0x2880;
	for(k=0;k<4;k++) {
		pstbl=0;
		for(i=0;i<ss_num;i++) {
			stbl=ssblt[i];
			if(stbl->bank!=k) continue;
			j=bank_next[k];
			if(j+stbl->len>0xffc0) { if(k==3) j=0x2880; else j=bank_next[0]; bank_lwr[k]=j; pstbl=0; }
			if(j+stbl->len>0x3c00 && j<0xd000) {
				SetCursor(normal_cursor);
				wsprintf(buffer,"Not enough space for music bank %d",k);
				MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
				doc->m_modf=1;
				return;
			}
			if(pstbl && (pstbl->flag&1) && (stbl->flag&2)) j--,pstbl->len--;
			stbl->addr=j;
			pstbl=stbl;
			bank_next[k]=j+stbl->len;
		}
	}
	for(i=0;i<ss_num;i++) {
		stbl=ssblt[i];
		for(j=stbl->relnum-1;j>=0;j--) {
			k=*(unsigned short*)(stbl->buf+stbl->relocs[j]);
			for(l=0;l<ss_num;l++) {
				sptbl=ssblt[l];
				if(sptbl->start<=k && sptbl->len>k-sptbl->start) goto noerror;
			}
			MessageBox(framewnd,"Internal error","Bad error happened",MB_OK);
			doc->m_modf=1;
			return;
noerror:
			if(((!sptbl->bank) && stbl->bank<3)||(sptbl->bank==stbl->bank)) {
				*(unsigned short*)(stbl->buf+stbl->relocs[j])=sptbl->addr+k-sptbl->start;
			} else {
				wsprintf(buffer,"An address outside the bank was referenced",sptbl->bank,stbl->bank);
				MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
				doc->m_modf=1;
				return;
			}
		}
	}
	l=m;
	for(k=0;k<4;k++) {
		switch(k) {
		case 1:
			rom[0x914]=l;
			rom[0x918]=(l>>8)|128;
			rom[0x91c]=l>>15;
			break;
		case 2:
			l=0xd8000;
			break;
		case 3:
			l=m;
			rom[0x932]=l;
			rom[0x936]=(l>>8)|128;
			rom[0x93a]=l>>15;
			break;
		}
		for(o=0;o<2;o++) {
			n=l+4;
			for(i=0;i<ss_num;i++) {
				stbl=ssblt[i];
				if(!stbl) continue;
				if((stbl->addr<0xd000)^o) continue;
				if(stbl->bank!=k) continue;
				if(n+stbl->len>((k==2)?0xdb7fc:0xd74fc)) {
					MessageBox(framewnd,"Not enough space for music","Bad error happened",MB_OK);
					doc->m_modf=1;
					return;
				}
				memcpy(rom+n,stbl->buf,stbl->len);
				n+=stbl->len;
				free(stbl->relocs);
				free(stbl->buf);
				free(stbl);
				ssblt[i]=0;
			}
			if(n>l+4) {
				*(short*)(rom+l)=n-l-4;
				*(short*)(rom+l+2)=o?bank_lwr[k]:0xd000;
				l=n;
			}
		}
		*(short*)(rom+l)=0;
		*(short*)(rom+l+2)=0x800;
		if(k==1) m=l+4;
	}
	free(ssblt);
	SetCursor(normal_cursor);
}

void Loadsongs(FDOC*doc)
{
	unsigned char*b,*c,*d;
	short*e;
	SONG*s,*s2;
	SONGPART*sp;
	SCMD*sc;
	ZWAVE*zw;
	int i,j,k,l=0,m,n,o,p,q,r,t,u;
	int range,filter;
	sc=doc->scmd=malloc(1024*sizeof(SCMD));
	doc->m_free=0;
	doc->m_size=1024;
	doc->srnum=0;
	doc->srsize=0;
	doc->sr=0;
	doc->sp_mark=0;
	b=doc->rom;
	sbank_ofs[1]=(b[0x91c]<<15)+((b[0x918]&127)<<8)+b[0x914];
	sbank_ofs[3]=(b[0x93a]<<15)+((b[0x936]&127)<<8)+b[0x932];
	for(i=0;i<1024;i++) sc[i].next=i+1,sc[i].prev=i-1;
	sc[1023].next=-1;
	for(i=0;i<3;i++) {
		b=Getspcaddr(doc,0xd000,i);
		for(j=0;;j++) if((r=((unsigned short*)b)[j])>=0xd000) {r=r-0xd000>>1; break;}
		doc->numsong[i]=r;
		for(j=0;j<r;j++) {
			k=((unsigned short*)b)[j];
			if(!k) doc->songs[l]=0; else {
				c=Getspcaddr(doc,k,i);
				if(!spcbank) m=0; else m=l-j;
				for(;m<l;m++) if(doc->songs[m] && doc->songs[m]->addr==k) {
					(doc->songs[l]=doc->songs[m])->inst++;
					break;
				}
				if(m==l) {
					doc->songs[l]=s=malloc(sizeof(SONG));
					s->inst=1;
					s->addr=k;
					s->flag=!spcbank;
					for(m=0;;m++)
						if((n=((unsigned short*)c)[m])<256) break;
					if(n>0) s->flag|=2,s->lopst=((unsigned short*)c)[m+1]-k>>1;
					s->numparts=m;
					s->tbl=malloc(4*m);
					for(m=0;m<s->numparts;m++) {
						k=((unsigned short*)c)[m];
						d=Getspcaddr(doc,k,i);
						if(!spcbank) n=0; else n=l-j;
						for(;n<l;n++) {
							s2=doc->songs[n];
							if(s2) for(o=0;o<s2->numparts;o++)
								if(s2->tbl[o]->addr==k) {
									(s->tbl[m]=s2->tbl[o])->inst++;
									goto foundpart;
								}
						}
						for(o=0;o<m;o++) if(s->tbl[o]->addr==k) {
							(s->tbl[m]=s->tbl[o])->inst++;
							goto foundpart;
						}
						sp=s->tbl[m]=malloc(sizeof(SONGPART));
						sp->flag=!spcbank;
						sp->inst=1;
						sp->addr=k;
						p=50000;
						for(o=0;o<8;o++) {
							q=sp->tbl[o]=Loadscmd(doc,((unsigned short*)d)[o],i,p);
							sc=doc->scmd+q;
							if((sc->flag&4) && sc->tim<p) p=sc->tim;
						}
foundpart:;
					}
				}
			}
			l++;
		}
	}
	b=Getspcaddr(doc,0x800,0);
	doc->snddat1=malloc(spclen);
	doc->sndlen1=spclen;
	memcpy(doc->snddat1,b,spclen);
	b=Getspcaddr(doc,0x17c0,0);
	doc->snddat2=malloc(spclen);
	doc->sndlen2=spclen;
	memcpy(doc->snddat2,b,spclen);
	b=Getspcaddr(doc,0x3d00,0);
	doc->insts=malloc(spclen);
	memcpy(doc->insts,b,spclen);
	doc->numinst=spclen/6;
	b=Getspcaddr(doc,0x3e00,0);
	doc->m_ofs=b-doc->rom+spclen;
	doc->sndinsts=malloc(spclen);
	memcpy(doc->sndinsts,b,spclen);
	doc->numsndinst=spclen/9;
	b=Getspcaddr(doc,0x3c00,0);
	zw=doc->waves=malloc(sizeof(ZWAVE)*(spclen>>2));
	p=spclen>>1;
	for(i=0;i<p;i+=2) {
		j=((unsigned short*)b)[i];
		if(j==65535) break;
		for(k=0;k<i;k+=2) {
			if(((unsigned short*)b)[k]==j) {
				zw->copy=k>>1;
				goto foundwave;
			}
		}
		zw->copy=-1;
foundwave:
		d=Getspcaddr(doc,j,0);
		e=malloc(2048);
		k=0;
		l=1024;
		u=t=0;
		for(;;) {
			m=*(d++);
			range=(m>>4)+8;
			filter=(m&12)>>2;
			for(n=0;n<8;n++) {
				o=(*d)>>4;
				if(o>7) o-=16;
				o<<=range;
				if(filter) o+=(t*fil1[filter]>>fil2[filter])-((u&-256)*fil3[filter]>>4);
				if(o>0x7fffff) o=0x7fffff;
				if(o<-0x800000) o=-0x800000;
				u=o;
//				if(t>0x7fffff) t=0x7fffff;
//				if(t<-0x800000) t=-0x800000;
				e[k++]=o>>8;
				o=*(d++)&15;
				if(o>7) o-=16;
				o<<=range;
				if(filter) o+=(u*fil1[filter]>>fil2[filter])-((t&-256)*fil3[filter]>>4);
				if(o>0x7fffff) o=0x7fffff;
				if(o<-0x800000) o=-0x800000;
				t=o;
//				if(u>0x7fffff) u=0x7fffff;
//				if(u<-0x800000) u=-0x800000;
				e[k++]=o>>8;
			}
			if(m&1) {zw->lflag=(m&2)>>1; break;}
			if(k==l) {l+=1024;e=realloc(e,l<<1);}
		}
		e=zw->buf=realloc(e,k+1<<1);
		zw->lopst=(((unsigned short*)b)[i+1]-j)*16/9;
		if(zw->lflag) e[k]=e[zw->lopst]; else e[k]=0;
		zw->end=k;
		zw++;
	}
	doc->numwave=i>>1;
	doc->m_loaded=1;
	doc->w_modf=0;
}
void Edittrack(FDOC*doc,short i)
{
	int j,k,l;
	SRANGE*sr=doc->sr;
	SCMD*sc;
	k=doc->srnum;
	sc=doc->scmd;
	if(i==-1) return;
	if(i>=doc->m_size) {wsprintf(buffer,"Invalid address: %04X",i);goto error;}
	for(;;) {if((j=sc[i].prev)!=-1) i=j; else break;}
	for(l=0;l<k;l++) if(sr->first==i) break; else sr++;
	if(l==k) {
		wsprintf(buffer,"Not found: %04X",i);
error:
		MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
		return;
	}
	if(sr->editor) SendMessage(clientwnd,WM_MDIACTIVATE,(int)(sr->editor),0);
	else Editwin(doc,"TRACKEDIT","Song editor",l+(i<<16),sizeof(TRACKEDIT));
}
HWAVEOUT hwo;
int sndinit=0;
int sounddev=0x10001;
WAVEHDR *wbufs;
int wcurbuf;
int wnumbufs;
int wbuflen;
FDOC*sounddoc=0;
SONG*playedsong;
int playedpatt;
int envdat[]={
	0,0x3ff00,0x54900,0x64d31,0x83216,0xaec33,0xc9a63,0x10624d,0x154725,
	0x199999,0x202020,0x2B1DA4,0x333333,0x3F03F0,0x563B48,
	0x667000,0x7E07E0,0xAAAAAA,0xCCCCCC,0x1000000,0x1555555,0x1999999,
	0x2000000,0x2AAAAAA,0x3333333,0x4000000,0x5555555,0x6670000,
	0x8000000,0xAAAAAAA,0x10000000,0x20000000
};

char midinst[16],midivol[16],midipan[16],midibank[16],midipbr[16];
short midipb[16];
ZMIXWAVE zwaves[8];
ZCHANNEL zchans[8];
short *wdata;
int*mixbuf;
int exitsound=0;
int songtim,songspd=100;
int songspdt=0,songspdd=0;
unsigned short globvol,globvolt,globvold;
short globtrans;
//CRITICAL_SECTION cs_song;
void Playpatt()
{
	int i;
	ZCHANNEL*zch=zchans;
	SONGPART*sp;
	sp=playedsong->tbl[playedpatt];
	for(i=0;i<8;i++) zch->playing=1,zch->tim=1,zch->loop=0,(zch++)->pos=sp->tbl[i];
}
const int freqvals[13]={0x157,0x16c,0x181,0x198,0x1b0,0x1ca,0x1e5,0x202,0x221,
   0x241,0x263,0x288,0x2ae};
const unsigned char nlength[8]={
	0x32,0x65,0x7f,0x98,0xb2,0xcb,0xe5,0xfc
};
const unsigned char nvol[16]={
	0x19,0x32,0x4c,0x65,0x72,0x7f,0x8c,0x98,0xa5,0xb2,0xbf,0xcb,0xd8,0xe5,0xf2,0xfc
};
unsigned char pbwayflag,volflag,pbstatflag,pbflag,fqflag,noteflag;
unsigned char activeflag=255,sndinterp=0;
void midinoteoff(ZCHANNEL*zch)
{
	if(zch->midnote!=0xff) {
		midiOutShortMsg(hwo,(0x80+zch->midch)|(zch->midnote<<8)|0x400000);
		if(zch->midnote&0xff00) midiOutShortMsg(hwo,(0x80+zch->midch)|(zch->midnote&0xff00)|0x400000);
	}
	zch->midnote=0xff;
}
void Stopsong()
{
	int i;
	ZMIXWAVE*zw=zwaves;
	sounddoc=0;
	if(sounddev<0x20000)
	for(i=0;i<8;i++) (zw++)->pflag=0;
	else for(i=0;i<8;i++) midinoteoff(zchans+i);
}
int mix_freq,mix_flags;
unsigned short midi_inst[0x19]={
	0x0050,0x0077,0x002f,0x0050,0x0051,0x001b,0x0051,0x0051,
	0x004d,0x0030,0x002c,0x0039,0x00b1,0x004f,0x000b,0x0004,
	0x10a5,0x0038,0x003c,0x00a8,0x00a8,0x0036,0x0048,0x0035,
	0x001a
};
unsigned short midi_trans[0x19]={
	0x00fb,0x0005,0x000c,0x0c00,0x0000,0x0002,0xfb02,0xf602,
	0x0027,0x0000,0x0000,0xf400,0x0024,0x0000,0x0018,0x000c,
	0x0020,0x0000,0x0000,0x0028,0x0028,0x000c,0x000c,0x0001,
	0x0000
};
int midi_timer;
int ws_freq=22050,ws_bufs=24,ws_len=256,ws_flags=3;
int miditimeres=5;
int ms_tim1=5,ms_tim2=20,ms_tim3=5;
void Updatesong()
{
	ZCHANNEL*zch;
	ZMIXWAVE*zw;
	SCMD*sc;
	ZINST*zi;
	int i,j,k,l,m;
	unsigned char chf;
	if(!(sounddoc&&playedsong)) return;
	if(sounddev<0x20000) songtim+=(songspd*wbuflen<<3-(mix_flags&1))/mix_freq;
	else songtim+=songspd*ms_tim2/20;
	for(;songtim>0;) {
		k=0;
		zch=zchans;
		zw=zwaves;
		chf=1;
		for(i=0;i<8;i++,zch++,zw++,chf<<=1) {
			if(!zch->playing) continue;
			zch->tim--;
			k=1;
			if(zch->ntim) {
				zch->ntim--;
				if(!zch->ntim) {
					l=zch->pos;
					j=zch->loop;
nexttime:
					if(l==-1) {
						j--;
						if(j<=0) goto endnote;
						l=zch->lopst;
						goto nexttime;
					}
					sc=sounddoc->scmd+l;
					if(sc->cmd!=0xc8) {
						if(sc->cmd==0xef) {
							l=*(short*)&(sc->p1);
							goto nexttime;
						}
						if(sc->cmd>=0xe0) {
							l=sc->next;
							goto nexttime;
						}
endnote:
						if(sounddev<0x20000) zw->envs=3;
						else midinoteoff(zch);
					}
				}
			}
			if(zch->volt) {
				zch->volt--;
				zch->vol+=zch->vold;
				volflag|=chf;
			}
			if(zch->pant) {
				zch->pant--;
				zch->pan+=zch->pand;
				volflag|=chf;
			}
			if(pbflag&chf) {
				if(!zch->pbt) {
					if(!zch->pbtim) zch->pbtim++;
					if(pbstatflag&chf) pbflag&=~chf; else {
						zch->pbt=zch->pbtim+1;
						if(sounddev<0x20000) {
							zch->note=j=zch->pbdst;
							l=j%12;
							j=freqvals[l]+(freqvals[l+1]-freqvals[l])*zch->ft<<(j/12+4);
							zch->fdlt=(j-zch->freq)/zch->pbtim;
						} else {
							j=zch->pbdst-zch->note;
							zch->midpbdlt=((j<<13)/midipbr[zch->midch]+0x2000-zch->midpb)/zch->pbtim;
						}
						pbstatflag|=chf;
					}
				} else if(pbstatflag&chf) {
					if(sounddev<0x20000)
						zch->freq+=zch->fdlt;
					else zch->midpb+=zch->midpbdlt;
					fqflag|=chf;
				}
				zch->pbt--;
			}
			if(zch->vibt) {
				zch->vibt--;
				zch->vibdep+=zch->vibdlt;
			}
			zch->vibcnt+=zch->vibspd;
			if(zch->vibdlt) fqflag|=chf;
			if(!zch->tim) {
again:
				if(zch->pos==-1) {zch->playing=0;continue;}
				sc=sounddoc->scmd+zch->pos;
				j=sc->next;
				if(j==-1 && zch->loop) {
					zch->loop--;
					if(!zch->loop) zch->pos=zch->callpos;
					else zch->pos=zch->lopst;
				} else {
					if(j>=sounddoc->m_size) zch->playing=0,zch->pos=-1;
					else zch->pos=j;
				}
				if(sc->flag&1) zch->t1=sc->b1;
				if(!zch->t1) zch->t1=255;
				if(sc->flag&2) {
					j=sc->b2;
					zch->nvol=nvol[j&15];
					zch->nlength=nlength[(j>>4)&7];
				}
				if(sc->cmd<0xc8) {
					zch->ftcopy=zch->ft;
					j=sc->cmd-0x80+zch->trans+globtrans;
					if(pbwayflag&chf) j-=zch->pbdlt;
					if(sounddev<0x20000) {
						zi=sounddoc->insts+zch->wnum;
						l=j%12;
						zch->freq=freqvals[l]+((freqvals[l+1]-freqvals[l])*zch->ft>>8)<<(j/12+4);
						zw->pos=0;
						zw->wnum=zi->samp;
						zw->envclk=0;
						if(zi->ad&128) {
							zw->atk=((zi->ad<<1)&31)+1;
							zw->dec=((zi->ad>>3)&14)+16;
							zw->sus=zi->sr>>5;
							zw->rel=zi->sr&31;
							zw->envs=0;
							zw->envx=0;
						} else {
							zw->sus=7;
							zw->rel=0;
							zw->atk=31;
							zw->dec=0;
							zw->envs=0;
						}
						zw->envclklo=0;
						zw->pflag=1;
					} else {
						midinoteoff(zch);
						if(activeflag&chf) {
							if(zch->wnum>=25) goto nonote;
							zch->midch=(midi_inst[zch->wnum]&0x80)?9:i;
							zch->midnum=zch->wnum;
							if(zch->midch<8) {
								if(midibank[zch->midch]!=midi_inst[zch->wnum]>>8) {
									midibank[zch->midch]=midi_inst[zch->wnum]>>8;
									midiOutShortMsg(hwo,0xb0+zch->midch|(midibank[zch->midch]<<16));
								}
								if(midinst[zch->midch]!=midi_inst[zch->wnum]) {
									midinst[zch->midch]=midi_inst[zch->wnum];
									midiOutShortMsg(hwo,0xc0+zch->midch|(midinst[zch->midch]<<8));
								}
								l=j+(char)(midi_trans[zch->wnum])+24;
								if(l>=0 && l<0x80) zch->midnote=l;
								if(midi_trans[zch->wnum]&0xff00) {
									l=j+(char)(midi_trans[zch->wnum]>>8)+24;
									if(l>=0 && l<0x80) zch->midnote|=l<<8;
								}
							} else {
								l=midi_inst[zch->wnum]&0x7f;
								if(midinst[zch->midch]!=midi_inst[zch->wnum]>>8) {
									midinst[zch->midch]=midi_inst[zch->wnum]>>8;
									midiOutShortMsg(hwo,0xc0+zch->midch|(midinst[zch->midch]<<8));
								}
								zch->midnote=l;
								zch->pbdst=(j<<1)-midi_trans[zch->wnum];
								zch->pbtim=0;
								pbflag|=chf;
							}
							zch->midpb=0x2000+(zch->ftcopy<<5)/midipbr[zch->midch];
							noteflag|=chf;
						}
					}
					fqflag|=chf;
					zch->pbt=zch->pbdly;
					zch->note=j;
					zch->vibt=zch->vibtim;
					zch->vibcnt=0;
					zch->vibdep=0;
					pbstatflag&=~chf;
					if(zch->pbdlt) pbflag|=chf,zch->pbdst=zch->pbdlt+zch->note;
					volflag|=chf;
nonote:;
				}
				if(sc->cmd==0xc9) {
					if(sounddev<0x20000) zw->envs=3; else midinoteoff(zch);
					pbflag&=~chf;
				}
				if(sc->cmd<0xe0) {
					zch->tim=zch->t1;
					zch->ntim=(zch->nlength*zch->tim)>>8;
					j=zch->pos;
					if(j!=-1) {
						sc=sounddoc->scmd+j;
						if(sc->cmd==249) {
							zch->pbt=sc->p1;
							zch->pbtim=sc->p2;
							zch->pbdst=sc->p3-128+zch->trans+globtrans;
							pbstatflag&=~chf;
							pbflag|=chf;
						}
						j=sc->next;
					}
					if(sounddev>=0x20000) {
						if(zch->midpb==0x2000 && (pbflag&chf)) {
							l=zch->pbdst-zch->note;
							if(l<0) l=-l;
							while(j!=-1) {
								sc=sounddoc->scmd+j;
								if(sc->cmd<0xe0 && sc->cmd==0xc8) break;
								if(sc->cmd==0xe9) {
									m=sc->p3-128+zch->trans+globtrans-zch->note;
									if(m<0) m=-m;
									if(m>l) l=m;
								}
								j=sc->next;
							}
							l+=2;
							if(midipbr[zch->midch]!=l) {
								midiOutShortMsg(hwo,0x6b0+zch->midch+(l<<16));
								midipbr[zch->midch]=l;
							}
						}
						if((pbflag&chf) && (!zch->pbtim)) {
							pbflag&=~chf;
							j=zch->pbdst-zch->note;
							zch->midpb=((j<<13)+(zch->ftcopy<<5))/midipbr[zch->midch]+0x2000;
						}
					}
				} else {
					switch(sc->cmd) {
					case 224:
						zch->wnum=sc->p1;
						break;
					case 225:
						zch->pan=sc->p1<<8;
						volflag|=chf;
						break;
					case 226:
						j=sc->p1+1;
						zch->pant=j;
						zch->pand=(((sc->p2<<8)-zch->pan)/j);
						break;
					case 227:
						zch->vibtim=sc->p1+1;
						zch->vibspd=sc->p2;
						zch->vibdlt=(sc->p3<<8)/zch->vibtim;
						break;
					case 229:
						globvol=sc->p1<<8;
						break;
					case 230:
						j=sc->p1+1;
						globvolt=j;
						globvold=((sc->p2<<8)-globvol)/j;
						volflag|=chf;
						break;
					case 231:
						songspd=sc->p1<<8;
						break;
					case 232:
						j=sc->p1+1;
						songspdt=j;
						songspdd=((sc->p2<<8)-songspd)/j;
						break;
					case 233:
						globtrans=(char)sc->p1;
						break;
					case 234:
						zch->trans=sc->p1;
						break;
					case 237:
						zch->vol=sc->p1<<8;
						volflag|=chf;
						break;
					case 238:
						j=sc->p1+1;
						zch->volt=j;
						zch->vold=((sc->p2<<8)-zch->vol)/j;
						volflag|=chf;
						break;
					case 239:
						if(*(unsigned short*)&(sc->p1)>=sounddoc->m_size) break;
						zch->callpos=zch->pos;
						zch->lopst=zch->pos=*(unsigned short*)&(sc->p1);
						if(zch->pos>=sounddoc->m_size) zch->playing=0,zch->pos=zch->lopst=0;
						zch->loop=sc->p3;
						break;
					case 241:
						zch->pbdly=sc->p1;
						zch->pbtim=sc->p2;
						zch->pbdlt=sc->p3;
						pbwayflag&=~chf;
						break;
					case 242:
						zch->pbdly=sc->p1;
						zch->pbtim=sc->p2;
						zch->pbdlt=sc->p3;
						pbwayflag|=chf;
						break;
					case 244:
						zch->ft=sc->p1;
						break;
					}
					goto again;
				}
			}
			if(volflag&chf) {
				volflag&=~chf;
				if(sounddev<0x20000) {
					zw->vol1=0x19999*(zch->nvol*zch->vol*((zch->pan>>8))>>22)>>16;
					zw->vol2=(0x19999*((zch->nvol*zch->vol*(20-((zch->pan>>8)))>>22))>>16);
				} else {
					l=((zch->vol>>9)*globvol>>16)*soundvol>>8;
					if(l!=midivol[zch->midch]) {
						midivol[zch->midch]=l;
						midiOutShortMsg(hwo,(0x7b0+zch->midch)|(l<<16));
					}
					l=((0x1400-zch->pan)*0x666)>>16;
					if(l!=midipan[zch->midch]) {
						midipan[zch->midch]=l;
						midiOutShortMsg(hwo,(0xab0+zch->midch)|(l<<16));
					}
				}
			}
			if(fqflag&chf) {
				fqflag&=~chf;
				if(sounddev<0x20000) {
					zi=sounddoc->insts+zch->wnum;
					l=zch->freq*((zi->multhi<<8)+zi->multlo)>>14;
					if(zch->vibdep) if(zch->vibcnt<128)
						l=l*(65536+(zch->vibdep*(zch->vibcnt-64)>>11))>>16;
					else l=l*(65536+(zch->vibdep*(191-zch->vibcnt)>>11))>>16;
					zw->freq=l;
				} else {
					l=zch->midpb;
					if(zch->vibdep) if(zch->vibcnt<128)
					l+=(zch->vibdep*(zch->vibcnt-64)>>10)/midipbr[zch->midch];
					else l+=(zch->vibdep*(191-zch->vibcnt)>>10)/midipbr[zch->midch];
					if(l!=midipb[zch->midch]) {
						midipb[zch->midch]=l;
						midiOutShortMsg(hwo,0xe0+zch->midch+((l&0x7f)<<8)+((l&0x3f80)<<9));
					}
				}
			}
			if(noteflag&chf) {
				noteflag&=~chf;
				if(zch->midnote!=0xff) {
					midiOutShortMsg(hwo,0x90+zch->midch|(zch->midnote<<8)|((zch->nvol<<15)&0x7f0000));
					if(zch->midnote&0xff00)
					midiOutShortMsg(hwo,0x90+zch->midch|(zch->midnote&0xff00)|((zch->nvol<<15)&0x7f0000));
				}
			}
		}
		if(songspdt) {
			songspdt--;
			songspd+=songspdd;
		}
		if(globvolt) {
			globvolt--;
			globvol+=globvold;
		}
		if(!k) {
			playedpatt++;
			if(playedpatt>=playedsong->numparts) if(playedsong->flag&2) playedpatt=playedsong->lopst; else {Stopsong();break;}
			if(playedpatt>=playedsong->numparts) {Stopsong();break;}
			Playpatt();
		} else songtim-=6100;
	}
}
void Playsong(FDOC*doc,int num)
{
	ZCHANNEL*zch=zchans;
	int i;
//	EnterCriticalSection(&cs_song);
	Stopsong();
	playedsong=doc->songs[num];
	for(i=0;i<8;i++) {
		zch->pand=zch->pant=zch->volt=zch->vold=zch->vibtim=0;
		zch->vibdep=zch->vibdlt=zch->pbdlt=0;
		zch->vol=65535;
		zch->pan=2048;
		zch->midpb=0x2000;
		zch->midch=i;
		zch->trans=0;
		zch->ft=0;
		zch->t1=255;
		zch++;
	}
	songspd=5500;
	songtim=0;
	songspdt=0;
	globvol=65535;
	globvolt=0;
	globtrans=0;
	playedpatt=0;
	volflag=pbflag=0;
	sounddoc=doc;
	Playpatt();
//	LeaveCriticalSection(&cs_song);
}
int soundthr=0;
void Mixbuffer()
{
	static int i,j,k,l,m,n,o;
	ZMIXWAVE*zmw=zwaves;
	ZWAVE*zw;
	short chf=1;
	int*b;
	short*c;
	unsigned char*d;
	static char v1,v2;
	static unsigned short f;
	static unsigned envx,envclk,envclklo,envclkadd,envmul;
//	if(soundthr) EnterCriticalSection(&cs_song);
	Updatesong();
	ZeroMemory(mixbuf,wbuflen<<2);
	envmul=(wbuflen<<16-(mix_flags&1))/mix_freq<<16;
	for(i=0;i<8;i++,zmw++,chf<<=1) {
		if(!(activeflag&chf)) continue;
		if(!zmw->pflag) continue;
		n=zmw->wnum;
		if(n<0 || n>=sounddoc->numwave) continue;
		zw=sounddoc->waves+n;
		b=mixbuf;
		j=zmw->pos;
		c=zw->buf;
		envx=zmw->envx;
		envclk=zmw->envclk;
		envclklo=zmw->envclklo;
		switch(zmw->envs) {
		case 0:
			envclkadd=envdat[zmw->atk];
			__asm mov eax,envclkadd
			__asm mov edx,envmul
			__asm imul edx
			__asm add envclklo,eax
			__asm adc edx,edx
			__asm add envx,edx
			if(envx>0xfe0000) {
				envx=0xfe0000;
				envclklo=0;
				if(zmw->sus==7) zmw->envs=2; else zmw->envs=1;
			}
			break;
		case 1:
			envclkadd=envdat[zmw->dec];
			__asm mov eax,envclkadd
			__asm mov edx,envmul
			__asm imul edx
			__asm add envclklo,eax
			__asm adc envclk,edx
			while(envclk>0x20000) envclk-=0x20000,envx-=envx>>8;
			if(envx<(zmw->sus<<21)) {
				envx=zmw->sus<<21;
				envclklo=0;
				zmw->envs=2;
			}
			break;
		case 2:
			envclkadd=envdat[zmw->rel];
			__asm mov eax,envclkadd
			__asm mov edx,envmul
			__asm imul edx
			__asm add envclklo,eax
			__asm adc envclk,edx
			while(envclk>0x20000) envclk-=0x20000,envx-=envx>>8;
			break;
		case 3:
			envclkadd=envdat[0x1f]>>1;
			__asm mov eax,envclkadd
			__asm mov edx,envmul
			__asm imul edx
			__asm add envclklo,eax
			__asm sbb envx,edx
			if(envx>=0x80000000) {
				zmw->pflag=0;
				envx=0;
			}
			break;
		}
		zmw->envx=envx;
		zmw->envclk=envclk;
		zmw->envclklo=envclklo;
		v1=((zmw->vol1*(envx>>16)>>8)*globvol>>16)*soundvol>>8;
		v2=((zmw->vol2*(envx>>16)>>8)*globvol>>16)*soundvol>>8;
		f=(zmw->freq<<12)/mix_freq;
		k=wbuflen;
		if(zw->lopst<zw->end) l=zw->lopst<<12; else l=0;
		m=zw->end<<12;
		if(!m) continue;
		if(mix_flags&1) {
			k>>=1;
			if(sndinterp) {
				if(zw->lflag) while(k--) {
					if(j>=m) j+=l-m;
					o=j>>12;
					o=c[o]+((c[o+1]-c[o])*(j&4095)>>12);
					*(b++)+=o*v1;
					*(b++)+=o*v2;
					j+=f;
				} else while(k--) {
					if(j>=m) {zmw->pflag=0; break;}
					o=j>>12;
					o=c[o]+((c[o+1]-c[o])*(j&4095)>>12);
					*(b++)+=o*v1;
					*(b++)+=o*v2;
					j+=f;
				}
			} else {
				if(zw->lflag) while(k--) {
					if(j>=m) j+=l-m;
					*(b++)+=c[j>>12]*v1;
					*(b++)+=c[j>>12]*v2;
					j+=f;
				} else while(k--) {
					if(j>=m) {zmw->pflag=0; break;}
					*(b++)+=c[j>>12]*v1;
					*(b++)+=c[j>>12]*v2;
					j+=f;
				}
			}
		} else {
			v1=v1+v2>>1;
			if(sndinterp) {
				if(zw->lflag) while(k--) {
					if(j>=m) j+=l-m;
					o=j>>12;
					o=c[o]+((c[o+1]-c[o])*(j&4095)>>12);
					*(b++)+=o*v1;
					j+=f;
				} else while(k--) {
					if(j>=m) {zmw->pflag=0; break;}
					o=j>>12;
					o=c[o]+((c[o+1]-c[o])*(j&4095)>>12);
					*(b++)+=o*v1;
					j+=f;
				}
			} else {
				if(zw->lflag) while(k--) {
					if(j>=m) j+=l-m;
					*(b++)+=c[j>>12]*v1;
					j+=f;
				} else while(k--) {
					if(j>=m) {zmw->pflag=0; break;}
					*(b++)+=c[j>>12]*v1;
					j+=f;
				}
			}
		}
		zmw->pos=j;
	}
//	if(soundthr) LeaveCriticalSection(&cs_song);
	k=wbuflen;
	b=mixbuf;
	if(mix_flags&2) {
		c=wdata+(wcurbuf*wbuflen);
		while(k--) *(c++)=(*(b++))>>7;
	} else {
		d=((LPSTR)wdata)+(wcurbuf*wbuflen);
		while(k--) *(d++)=((*(b++))>>15)^128;
	}
}
HANDLE wave_end;

void CALLBACK midifunc(UINT timerid,UINT msg,DWORD inst,DWORD p1,DWORD p2)
{
	if(exitsound) {
//		if(exitsound==1) {
//			timeKillEvent(midi_timer);
//			exitsound=2;
//			SetEvent(wave_end);
//		}
		return;
	}
	Updatesong();
}
void CALLBACK midifunc2(HWND win,UINT msg,UINT timerid,DWORD systime)
{
	if(exitsound) return;
	Updatesong();
}
int CALLBACK soundproc(HWAVEOUT bah,UINT msg,DWORD inst,DWORD p1,DWORD p2)
{
	int r;
	switch(msg) {
	case WOM_DONE:
		r=((WAVEHDR*)p1)->dwUser;
		if(exitsound) {
			if(exitsound==1) {
				exitsound=2;
				if(wave_end) SetEvent(wave_end);
			}
			return 0;
		}
		wcurbuf=r;
		if(r==wcurbuf) {
			while(wbufs[wcurbuf].dwFlags&WHDR_DONE) {
				Mixbuffer();
				wbufs[wcurbuf].dwFlags&=~WHDR_DONE;
				waveOutWrite(hwo,wbufs+wcurbuf,sizeof(WAVEHDR));
				wcurbuf=wcurbuf+1;
				if(wcurbuf==wnumbufs) wcurbuf=0;
			}
		}
	}
}
int CALLBACK soundfunc(LPVOID param)
{
	MSG msg;
	while(GetMessage(&msg,0,0,0)) {
		switch(msg.message) {
		case MM_WOM_DONE:
			soundproc(0,WOM_DONE,0,msg.lParam,0);
			break;
		}
	}
	return 0;
}
void Exitsound()
{
	int n;
	WAVEHDR*wh;
	if(!sndinit) return;
	exitsound=1;
	if(sounddev<0x20000) {
		SetCursor(wait_cursor);
		if(soundthr) {
			WaitForSingleObject(wave_end,INFINITE);
		} else {
			while(WaitForSingleObject(wave_end,50)==WAIT_TIMEOUT);
		}
		SetCursor(normal_cursor);
		waveOutReset(hwo);
		wh=wbufs;
		for(n=0;n<wnumbufs;n++,wh++) waveOutUnprepareHeader(hwo,wh,sizeof(WAVEHDR));
		waveOutClose(hwo);
		CloseHandle(wave_end);
		free(wbufs);
		free(wdata);
		free(mixbuf);
	} else {
		if(wver) {
			KillTimer(framewnd,midi_timer);
		} else {
			timeKillEvent(midi_timer);
			timeEndPeriod(miditimeres);
		}
		for(n=0;n<8;n++) midinoteoff(zchans+n);
		midiOutClose(hwo);
	}
//	DeleteCriticalSection(&cs_song);
	exitsound=0;
	sndinit=0;
	soundthr=0;
}
/*
#pragma code_seg("seg2")
#pragma data_seg("seg3")
BOOL (WINAPI*postmsgfunc)(HWND,UINT,WPARAM,LPARAM);
void CALLBACK testfunc(UINT timerid,UINT msg,DWORD inst,DWORD p1,DWORD p2)
{
	postmsgfunc(framewnd,4100,0,0);
}
#pragma code_seg()
#pragma data_seg()*/
void Initsound()
{
	int n,sh;
	WAVEFORMATEX wfx;
	WAVEHDR*wh;
	char*err;
	const unsigned char blkal[4]={1,2,2,4};
	if(sndinit) Exitsound();
	if((sounddev>>16)==2) {
		if(n=midiOutOpen(&hwo,sounddev-0x20001,0,0,0)) goto openerror;
		for(n=0;n<16;n++) {
			midinst[n]=midivol[n]=midipan[n]=midibank[n]=255;
			midipbr[n]=2;
			midipb[n]=65535;
			midiOutShortMsg(hwo,0x64b0+n);
			midiOutShortMsg(hwo,0x65b0+n);
			midiOutShortMsg(hwo,0x206b0+n);
		}
		noteflag=0;
		for(n=0;n<8;n++) zchans[n].midnote=0xff;
		if(wver) {
//			miditimeres=ms_tim1;
//			timeBeginPeriod(miditimeres);
//			midi_timer=timeSetEvent(ms_tim2,ms_tim3,testfunc,0,TIME_PERIODIC);
			midi_timer=SetTimer(framewnd,1,ms_tim2,(TIMERPROC)midifunc2);
		} else {
			miditimeres=ms_tim1;
			timeBeginPeriod(miditimeres);
			midi_timer=timeSetEvent(ms_tim2,ms_tim3,midifunc,0,TIME_PERIODIC);
			soundthr=1;
		}
		if(!midi_timer) {
			wsprintf(buffer,"Can't initialize timer: %08X",GetLastError());
			MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
			if(!wver) timeEndPeriod(miditimeres);
			midiOutClose(hwo);
			return;
		}
//		InitializeCriticalSection(&cs_song);
		sndinit=1;
		goto endinit;
	}
	soundthr=0;
//	ht=CreateThread(0,0,soundfunc,0,0,&sndthread);
//	if(ht) CloseHandle(ht);
	wbuflen=ws_len<<(ws_flags&1);
	sh=(ws_flags>>1);
	wfx.wFormatTag=1;
	wfx.nChannels=(ws_flags&1)+1;
	wfx.nSamplesPerSec=ws_freq;
	wfx.nAvgBytesPerSec=ws_freq<<sh+((ws_flags&2)>>1);
	wfx.nBlockAlign=blkal[ws_flags];
	wfx.wBitsPerSample=(ws_flags&2)?16:8;
	wfx.cbSize=0;
	wnumbufs=ws_bufs;
//	if(ht) n=waveOutOpen(&hwo,sounddev-0x10002,&wfx,sndthread,0,CALLBACK_THREAD),soundthr=1;
/*	else*/ n=waveOutOpen(&hwo,sounddev-0x10002,&wfx,(int)soundproc,0,CALLBACK_FUNCTION);
	if(n) {
openerror:
		switch(n) {
		case MIDIERR_NODEVICE:
			err="No MIDI port was found.";
			break;
		case MMSYSERR_ALLOCATED:
			err="Already in use";
			break;
		case MMSYSERR_BADDEVICEID:
			err="The device was removed";
			break;
		case MMSYSERR_NODRIVER:
			err="No driver found";
			break;
		case MMSYSERR_NOMEM:
			err="Not enough memory";
			break;
		case WAVERR_BADFORMAT:
			err="Unsupported playback quality";
			break;
		case WAVERR_SYNC:
			err="It is synchronous";
			break;
		default:
			wsprintf(buffer,"Unknown error %08X",n);
			err=buffer+256;
			break;
		}
		wsprintf(buffer,"Cannot initialize sound (%s)",err);
		MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
		return;
	}
//	InitializeCriticalSection(&cs_song);
	wh=wbufs=malloc(sizeof(WAVEHDR)*ws_bufs);
	wdata=malloc(wbuflen*ws_bufs<<sh);
	for(n=0;n<8;n++) zwaves[n].pflag=0;
	for(n=0;n<wnumbufs;n++) {
		wh->lpData=(LPSTR)(wdata)+(n*wbuflen<<sh);
		wh->dwBufferLength=wbuflen<<sh;
		wh->dwFlags=0;
		wh->dwUser=n;
		waveOutPrepareHeader(hwo,wh,sizeof(WAVEHDR));
		wh++;
	}
	sndinit=1;
	mixbuf=malloc(wbuflen*ws_bufs<<2);
	mix_freq=ws_freq/6;
	mix_flags=ws_flags;
	for(wcurbuf=0;wcurbuf<wnumbufs;wcurbuf++)
		Mixbuffer();
	wcurbuf=0;
	for(n=0;n<wnumbufs;n++)
		waveOutWrite(hwo,wbufs+n,sizeof(WAVEHDR));
endinit:
	wave_end=CreateEvent(0,0,0,0);
}


short AllocScmd(FDOC*doc)
{
	int i=doc->m_free;
	int j,k;
	SCMD*sc;
	if(i==-1) {
		j=doc->m_size;
		doc->m_size+=1024;
		sc=doc->scmd=realloc(doc->scmd,doc->m_size*sizeof(SCMD));
		k=1023;
		while(k--) sc[j].next=j+1,j++;
		sc[j].next=-1;
		k=1023;
		while(k--) sc[j].prev=j-1,j--;
		sc[j].prev=-1;
		i=j;
	} else sc=doc->scmd;
	doc->m_free=sc[doc->m_free].next;
	if(doc->m_free!=-1) sc[doc->m_free].prev=-1;
	return i;
}

void NewSR(FDOC*doc,int bank)
{
	SCMD*sc;
	SRANGE*sr;
	if(doc->srnum==doc->srsize) {
		doc->srsize+=16;
		doc->sr=realloc(doc->sr,doc->srsize*sizeof(SRANGE));
	}
	sr=doc->sr+doc->srnum;
	doc->srnum++;
	sr->first=AllocScmd(doc);
	sr->bank=bank;
	sr->editor=0;
	sc=doc->scmd+sr->first;
	sc->prev=-1;
	sc->next=-1;
	sc->cmd=128;
	sc->flag=0;
	Edittrack(doc,sr->first);
}
void Loadtext(FDOC*doc)
{
	int i=0,j=0xe0000,k,l,m=64;
	unsigned char a;
	unsigned char*rom=doc->rom;
	unsigned char*b;
	int bd,bs;
	doc->tbuf=malloc(256);
	for(;;) {
		bs=128;
		bd=2;
		b=malloc(128);
		for(;;) {
			a=rom[j];
			if(a==0x80) j=0x75f40;
			else if(a==0xff) {doc->t_number=i;doc->t_loaded=1;return;}
			else if(a<0x67) j++,b[bd++]=a;
			else if(a>=0x88) {
				k=*(short*)(rom+0x745f3+(a<<1));
				l=*(short*)(rom+0x745f5+(a<<1));
				while(k<l) {
					b[bd++]=rom[0x78000+k];
					k++;
				}
				j++;
			} else if(a==0x7f) {j++;break;} else {
				l=rom[0x7536b+a];
				while(l--) b[bd++]=rom[j++];
			}
			if(bd>=bs-64) bs+=128,b=realloc(b,bs);
		}
		*(short*)b=bd;
		b=realloc(b,bd);
		if(i==m) m+=64,doc->tbuf=realloc(doc->tbuf,m<<2);
		doc->tbuf[i++]=b;
	}
}
void Savetext(FDOC*doc)
{
	int i,bd,j,k;
	short l,m,n,o,p,q,r,v,t,u,w;
	int s=0xe0000;
	unsigned char*b,b2[2048];
	unsigned char*rom=doc->rom;
	if(!doc->t_modf) return;
	doc->t_modf=0;
	w=(*(unsigned short*)(rom+0x74703))-0xc705>>1;
	for(i=0;i<doc->t_number;i++) {
		b=doc->tbuf[i];
		m=bd=0;
		k=*(short*)b;
		j=2;
		r=w;
		for(;j<k;)
		{
			q=b2[bd++]=b[j++];
			if(q>=0x67) {
				l=rom[0x7536b+q]-1;
				while(l--) b2[bd++]=b[j++];
				m=bd;
			}
			if(bd>m+1) {
				o=*(short*)(rom+0x74703);
				v=255;
				t=w;
				for(l=0;l<w;l++) {
					n=o;
					o=*(short*)(rom+0x74705+(l<<1));
					p=o-n-bd+m;
					if(p>=0) if(!memcmp(rom+0x78000+n,b2+m,bd-m)) {
						if(p<v) t=l,v=p;
						if(!p) {r=t;u=j;break;}
					}
				}
				if(t==w || b[j]>=0x67) {
					if(r!=w) {
						b2[m]=r+0x88;
						m++;
						j=u;
						bd=m;
						r=w;
					} else m=bd-1;
				}
			}
		}
		s+=bd;
		if(s<0xe0000 && s>0x77400) {
			doc->t_modf=1;
			MessageBox(framewnd,"Not enough space for monologue.","Bad error happened",MB_OK);
			return;
		}
		if(s>0xe8000) {
			rom[s-bd]=0x80;
			s=0x75f40+bd;
		}
		memcpy(rom+s-bd,b2,bd);
		rom[s++]=0x7f;
	}
	rom[s]=0xff;
	doc->modf=1;
}
const char z_alphabet[]=
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789!?-.,\0>()@£${}\"\0\0\0\0'\0\0\0\0\0\0\0 <\0\0\0\0";
const static char*tsym_str[]={
	"Up",
	"Down",
	"Left",
	"Right",
	0,
	"1HeartL",
	"1HeartR",
	"2HeartL",
	"3HeartL",
	"3HeartR",
	"4HeartL",
	"4HeartR",
	0,0,
	"A",
	"B",
	"X",
	"Y"
};
const static char*tcmd_str[]={
	"NextPic",
	"Choose",
	"Item",
	"Name",
	"Window",
	"Number",
	"Position",
	"ScrollSpd",
	"Selchg",
	"Crash",
	"Choose3",
	"Choose2",
	"Scroll",
	"1",
	"2",
	"3",
	"Color",
	"Wait",
	"Sound",
	"Speed",
	"Mark",
	"Mark2",
	"Clear",
	"Waitkey",
};
void Makeasciistring(FDOC*doc,unsigned char*buf,unsigned char*buf2,int bufsize)
{
	int i;
	short j,k,l,m,n;
	j=*(short*)buf2;
	l=2;
	for(i=0;i<bufsize-1;) {
		if(l>=j) break;
		k=buf2[l++];
		if(k<0x5f) {
			if(!z_alphabet[k]) {
				if(k==0x43) m=wsprintf(buffer,"...");
				else m=wsprintf(buffer,"[%s]",tsym_str[k-0x4d]);
				goto longstring;
			} else buf[i++]=z_alphabet[k];
		}
		else if(k>=0x67 && k<0x7f) {
			m=wsprintf(buffer,"[%s",tcmd_str[k-0x67]);
			n=doc->rom[0x7536b+k]-1;
			while(n--) m+=wsprintf(buffer+m," %02X",buf2[l++]);
			buffer[m++]=']';
longstring:
			n=0;
			while(m--) {
				buf[i++]=buffer[n++];
				if(i==bufsize-1) break;
			}
		}
		else { m=wsprintf(buffer,"[%02X]",k); goto longstring; }
	}
	buf[i]=0;
}
char*text_error;
char*Makezeldastring(FDOC*doc,char*buf)
{
	char*b2=malloc(128);
	int bd=2,bs=128;
	short j,l,m,k;
	char*n;
	for(;;)
	{
		j=*(buf++);
		if(j=='[') {
			m=strcspn(buf," ]");
			for(l=0;l<18;l++) if(tsym_str[l] && (!tsym_str[l][m]) && !_strnicmp(buf,tsym_str[l],m)) break;
			if(l==18) {
				for(l=0;l<24;l++) if((!tcmd_str[l][m]) && !_strnicmp(buf,tcmd_str[l],m)) break;
				if(l==24) {
					j=strtol(buf,&n,16);
					k=n-buf;
					if(k>2 || k<1) {
						buf[m]=0;
						wsprintf(buffer,"Invalid command \"%s\"",buf);
error:
						MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
						free(b2);
						text_error=buf;
						return 0;
					};
					m=k;
					b2[bd++]=j;
					l=0;
				} else b2[bd++]=l+0x67,l=doc->rom[0x753d2+l]-1;
			} else b2[bd++]=l+0x4d,l=0;
			buf+=m;
			while(l--) {
				if(*buf!=' ') {
syntaxerror:
					wsprintf(buffer,"Syntax error: '%c'",*buf);
					goto error;
				}
				buf++;
				j=strtol(buf,&n,16);
				m=n-buf;
				if(m>2 || m<1) {
					wsprintf(buffer,"Invalid number");
					goto error;
				};
				buf+=m;
				b2[bd++]=j;
			}
			if(*buf!=']') goto syntaxerror;
			buf++;
		} else {
			if(!j) break;
			for(l=0;l<0x5f;l++) if(z_alphabet[l]==j) break;
			if(l==0x5f) {
				wsprintf(buffer,"Invalid character '%c'",j);
				goto error;
			}
			b2[bd++]=l;
		}
		if(bd>bs-64) {
			bs+=128;
			b2=realloc(b2,bs);
		}
	}
	*(short*)b2=bd;
	return b2;
}
void Removepatches(FDOC*doc)
{
	int j,k;
	PATCH*p;
	j=doc->numpatch;
	p=doc->patches;
	for(k=0;k<j;k++) {
		memcpy(doc->rom+p->addr,p->pv,p->len);
		free(p->pv);
		p++;
	}
	for(k=0;k<doc->numseg;k++) {
		Changesize(doc,676+k,0);
	}
	doc->numpatch=0;
	free(doc->patches);
	doc->patches=0;
	doc->numseg=0;
}
char asmpath[MAX_PATH];
HACCEL actab;
void ProcessMessage(MSG*msg)
{
	RECT rc;
	SDCREATE *sdc;
	if(!TranslateMDISysAccel(clientwnd,msg)) {
		if(!TranslateAccelerator(framewnd,actab,msg)) {
			if(msg->message==WM_MOUSEMOVE) {
				GetWindowRect(msg->hwnd,&rc);
				mouse_x=LOWORD(msg->lParam)+rc.left;
				mouse_y=HIWORD(msg->lParam)+rc.top;		
			}
			for(sdc=firstdlg;sdc;sdc=sdc->next)
				if(IsDialogMessage(sdc->win,msg)) break;
			if(!sdc) {
				TranslateMessage(msg);
				DispatchMessage(msg);
			}
		}
	}
}
BOOL CALLBACK errorsproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	HANDLE h;
	int i,j;
	char*b;
	switch(msg) {
	case WM_INITDIALOG:
		h=CreateFile("HMAGIC.ERR",GENERIC_READ,0,0,OPEN_EXISTING,FILE_FLAG_DELETE_ON_CLOSE,0);
		if((int)h==-1) break;
		i=GetFileSize(h,0);
		b=malloc(i+1);
		ReadFile(h,b,i,&j,0);
		CloseHandle(h);
		b[i]=0;
		SetDlgItemText(win,IDC_EDIT1,b);
		free(b);
		break;
	case WM_COMMAND:
		if(wparam==IDCANCEL) EndDialog(win,0);
	}
	return 0;
}
int Buildpatches(FDOC*doc)
{
	int i,j,k,l,q,r,s;
	ASMHACK*mod;
	MSG msg;
	PATCH*p;
	HANDLE h,h2,h3[2];
	FILETIME tim,tim2;
	SECURITY_ATTRIBUTES sa;
	STARTUPINFO sti;
	PROCESS_INFORMATION pinfo;
	char*m,*t;
	int*n,*o=0;
	j=doc->nummod;
	mod=doc->modules;
	Removepatches(doc);
	if(!j) {
nomod:
		MessageBox(framewnd,"No modules are loaded","Bad error happened",MB_OK);
		return 0;
	}
	k=wsprintf(buffer,"\"%s\"",asmpath);
	l=0;
	for(i=0;i<j;i++,mod++) {
		if(mod->flag&1) continue;
		l++;
		t=buffer+k;
		k+=wsprintf(buffer+k," \"%s\"",mod->filename);
		m=strrchr(t,'.');
		if(!m) continue;
		if(strrchr(t,'\\')>m) continue;
		if(stricmp(m,".asm")) continue;
		h=CreateFile(mod->filename,GENERIC_READ,0,0,OPEN_EXISTING,0,0);
		if(h==(HANDLE)-1) {
			wsprintf(buffer,"Unable to open %s",mod->filename);
			MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
			return 1;
		}
		GetFileTime(h,0,0,&tim);
		CloseHandle(h);
		*(int*)m='jbo.';
		h=CreateFile(mod->filename,GENERIC_READ,0,0,OPEN_EXISTING,0,0);
		if(h==(HANDLE)-1) goto buildnew;
		GetFileTime(h,0,0,&tim2);
		CloseHandle(h);
		__asm {
			mov eax,tim.dwLowDateTime
			mov edx,tim.dwHighDateTime
			sub eax,tim2.dwLowDateTime
			sbb edx,tim2.dwHighDateTime
			jb nonew
		}
buildnew:
		*(int*)m='msa.';
nonew:;
	}
	if(!l) goto nomod;
	sa.nLength=12;
	sa.lpSecurityDescriptor=0;
	sa.bInheritHandle=1;
	h=CreateFileMapping((HANDLE)-1,&sa,PAGE_READWRITE,0,4096,0);
	if(!h) {
nomem:
		MessageBox(framewnd,"Not enough memory.","Bad error happened",MB_OK);
		return 1;
	}
	n=MapViewOfFile(h,FILE_MAP_WRITE,0,0,0);
	if(!n) {
		CloseHandle(h);
		goto nomem;
	}
	wsprintf(buffer+k," -l -h $%X -o HMTEMP.DAT",h);
	h2=CreateFile("HMAGIC.ERR",GENERIC_WRITE,0,&sa,CREATE_ALWAYS,0,0);
	sti.cb=sizeof(sti);
	sti.lpReserved=0;
	sti.lpDesktop=0;
	sti.lpTitle=0;
	sti.dwFlags=STARTF_USESTDHANDLES;
	sti.cbReserved2=0;
	sti.lpReserved2=0;
	sti.hStdInput=(HANDLE)-1;
	sti.hStdOutput=h2;
	sti.hStdError=h2;
	n[0]=(int)CreateEvent(&sa,0,0,0);
	n[1]=(int)CreateEvent(&sa,0,0,0);
	if(!CreateProcess(0,buffer,0,0,1,DETACHED_PROCESS,0,0,&sti,&pinfo)) {
		CloseHandle(h2);
		UnmapViewOfFile(n);
		CloseHandle(h);
		wsprintf(buffer,"Unable to start %s",asmpath);
		MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
		return 1;
	}
	CloseHandle(h2);
	j=0;
	k=0;
	l=0;
	h3[0]=(void*)(n[0]);
	h3[1]=pinfo.hProcess;
	for(;;) {
		switch(MsgWaitForMultipleObjects(2,h3,0,INFINITE,QS_ALLEVENTS)) {
		case WAIT_OBJECT_0:
			switch(n[2]) {
			case 0:
				if(n[3]>=3) goto error;
				if(doc->numseg==92) {
					MessageBox(framewnd,"Too many segments","Bad error happened",MB_OK);
					break;
				}
				doc->numseg++;
				doc->segs[k]=0;
				i=Changesize(doc,0x802a4+k,n[4]);
				n[3]=cpuaddr(i);
				k++;
				if(!i) goto error;
				goto regseg;
			case 1:
				if(doc->patches) goto error;
				i=n[6];
				if(n[3]<3) {
					i=romaddr(i);
					if(i>=0x100000) {error:n[2]=1;break;}
					l++;
regseg:
					if(!(j&255)) o=realloc(o,j+512<<2);
					o[j]=i;
					o[j+1]=n[4];
					o[j+2]=n[5];
					o[j+3]=n[2];
					j+=4;
				}
				n[2]=0;
				break;
			case 2:
				doc->patches=malloc(l*sizeof(PATCH));
				for(i=0;i<j;i++) {
					doc->patches[i].len=0;
					doc->patches[i].addr=0;
					doc->patches[i].pv=0;
				}
				break;
			default:
				n[2]=1;
			}
			SetEvent((HANDLE)n[1]);
			break;
		case WAIT_OBJECT_0+1:
			goto done;
		case WAIT_OBJECT_0+2:
			while(PeekMessage(&msg,0,0,0,PM_REMOVE)) {
				if(msg.message==WM_QUIT) break;
				ProcessMessage(&msg);
			}
		}
	}
done:
	GetExitCodeProcess(pinfo.hProcess,&q);
	if(q) {
		DialogBoxParam(hinstance,(LPSTR)IDD_DIALOG23,framewnd,errorsproc,0);
errors:
		Removepatches(doc);
	} else {
		DeleteFile("HMAGIC.ERR");
		h2=CreateFile("HMTEMP.DAT",GENERIC_READ,0,0,OPEN_EXISTING,FILE_FLAG_DELETE_ON_CLOSE,0);
		if(h2==(HANDLE)-1) {
			MessageBox(framewnd,"Unable to apply patch","Bad error happened",MB_OK);
			q=1;
			goto errors;
		}
		p=doc->patches=malloc(l*sizeof(PATCH));
		doc->numpatch=l;
		s=j;
		j>>=2;
		for(r=0;r<j;r++) {
			for(i=0;i<s;i+=4)
				if(o[i+2]==r) break;
			if(o[i+3]) {				
				p->addr=o[i];
				p->len=o[i+1];
				p->pv=malloc(p->len);
				memcpy(p->pv,doc->rom+p->addr,p->len);
				p++;
			}
			ReadFile(h2,doc->rom+o[i],o[i+1],&l,0);
		}
		CloseHandle(h2);
	}
	free(o);
	CloseHandle(pinfo.hThread);
	CloseHandle(pinfo.hProcess);
	UnmapViewOfFile(n);
	CloseHandle(h);
	if(!q) {
		GetSystemTimeAsFileTime(&(doc->lastbuild));
		doc->modf=1;
		doc->p_modf=0;
	}
	return q;
}
BOOL CALLBACK patchdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	PATCHLOAD*ed;
	ASMHACK*mod;
	OPENFILENAME ofn;
	FDOC*doc;
	HWND hc;
	int i,j;
	char patchname[MAX_PATH];
	switch(msg) {
	case WM_INITDIALOG:
		SetWindowLong(win,DWL_USER,lparam);
		ed=(PATCHLOAD*)lparam;
		ed->dlg=win;
		doc=ed->ew.doc;
		mod=doc->modules;
		hc=GetDlgItem(win,3000);
		j=doc->nummod;
		for(i=0;i<j;i++,mod++)
			SendDlgItemMessage(win,3000,LB_ADDSTRING,0,(int)(doc->modules[i].filename));
		break;
	case WM_COMMAND:
		ed=(PATCHLOAD*)GetWindowLong(win,DWL_USER);
		doc=ed->ew.doc;
		switch(wparam) {
		case 3002:
			ofn.lStructSize=sizeof(ofn);
			ofn.hwndOwner=win;
			ofn.hInstance=hinstance;
			ofn.lpstrFilter="FSNASM source files\0*.ASM\0FSNASM module files\0*.OBJ\0";
			ofn.lpstrCustomFilter=0;
			ofn.nFilterIndex=1;
			ofn.lpstrFile=patchname;
			*patchname=0;
			ofn.nMaxFile=MAX_PATH;
			ofn.lpstrFileTitle=0;
			ofn.lpstrInitialDir=0;
			ofn.lpstrTitle="Load patch";
			ofn.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
			ofn.lpstrDefExt=0;
			ofn.lpfnHook=0;
			if(!GetOpenFileName(&ofn)) break;
			doc->nummod++;
			doc->modules=realloc(doc->modules,sizeof(ASMHACK)*doc->nummod);
			mod=doc->modules+doc->nummod-1;
			mod->filename=_strdup(patchname);
			mod->flag=0;
			SendDlgItemMessage(win,3000,LB_ADDSTRING,0,(int)patchname);
			doc->p_modf=1;
			break;
		case 3003:
			i=SendDlgItemMessage(win,3000,LB_GETCURSEL,0,0);
			if(i==-1) break;
			SendDlgItemMessage(win,3000,LB_DELETESTRING,0,(int)patchname);
			doc->nummod--;
			memcpy(doc->modules+i,doc->modules+i+1,(doc->nummod-i)*sizeof(ASMHACK));
			doc->modules=realloc(doc->modules,sizeof(ASMHACK)*doc->nummod);
			doc->p_modf=1;
			break;
		case 3004:
			Buildpatches(doc);
			break;
		}
	}
	return 0;
}
BOOL CALLBACK textdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	TEXTEDIT*ed;
	FDOC*doc;
	int i,j,k,l,m;
	char*b,*c;
	unsigned char*rom;
	HWND hc;
	switch(msg) {
	case WM_INITDIALOG:
		SetWindowLong(win,DWL_USER,lparam);
		ed=(TEXTEDIT*)lparam;
		CheckDlgButton(win,3003,BST_CHECKED);
		ed->dlg=win;
		ed->num=0;
		doc=ed->ew.doc;
		if(!doc->t_loaded) Loadtext(doc);
updstrings:
		hc=GetDlgItem(win,3000);
		b=malloc(256);
		if(ed->num) {
			rom=doc->rom;
			j=(*(unsigned short*)(rom+0x74703))-0xc705>>1;
			l=*((short*)(rom+0x74703));
			for(i=0;i<j;i++) {
				k=((short*)(rom+0x74705))[i];
				memcpy(b+130,rom+l+0x78000,k-l);
				*(short*)(b+128)=k-l+2;
				Makeasciistring(doc,b,b+128,128);
				wsprintf(buffer,"%03d: %s",i,b);
				SendMessage(hc,LB_ADDSTRING,0,(long)buffer);
				l=k;
			}
		} else for(i=0;i<doc->t_number;i++) {
			Makeasciistring(doc,b,doc->tbuf[i],256);
			wsprintf(buffer,"%03d: %s",i,b);
			SendMessage(hc,LB_ADDSTRING,0,(long)buffer);
		}
		free(b);
		break;
	case WM_CLOSE:
		return 1;
	case WM_COMMAND:
		ed=(TEXTEDIT*)GetWindowLong(win,DWL_USER);
		if(!ed) break;
		doc=ed->ew.doc;
		rom=doc->rom;
		switch(wparam) {
		case 3000|(LBN_DBLCLK<<16):
			i=SendMessage((HWND)lparam,LB_GETCURSEL,0,0);
			if(i!=-1) {
				b=malloc(2048);
				if(ed->num) {
					k=((short*)(rom+0x74705))[i];
					l=((short*)(rom+0x74703))[i];
					memcpy(b+1026,rom+l+0x78000,k-l);
					*(short*)(b+1024)=k-l+2;
					Makeasciistring(doc,b,b+1024,1024);
				} else Makeasciistring(doc,b,doc->tbuf[i],2048);
				SetDlgItemText(win,3001,b);
				free(b);
			} else SetDlgItemText(win,3001,0);
			break;
		case 3002:
			i=SendDlgItemMessage(win,3000,LB_GETCURSEL,0,0);
			if(i!=-1) {
				b=malloc(2048);
				GetDlgItemText(win,3001,b,2048);
				c=Makezeldastring(doc,b);
				hc=GetDlgItem(win,3000);
				if(c) {
					if(ed->num) {
						m=(*(short*)c)-2;
						j=*(unsigned short*)(rom+0x77ffe+*(short*)(rom+0x74703));
						k=((unsigned short*)(rom+0x74705))[i];
						l=((unsigned short*)(rom+0x74703))[i];
						if(j+m+l-k>0xc8d9) {
							MessageBeep(0);
							free(c);
							free(b);
							break;
						}
						memcpy(rom+0x68000+l+m,rom+0x68000+k,j-k);
						memcpy(rom+0x68000+l,c+2,m);
						k-=l;
						l=*(unsigned short*)(rom+0x74703)-0xc703>>1;
						for(j=i+1;j<l;j++) ((short*)(rom+0x74703))[j]+=m-k;
					} else {
						free(doc->tbuf[i]);
						doc->tbuf[i]=c;
					}
					Makeasciistring(doc,b,c,256);
					wsprintf(buffer,"%03d: %s",i,b);
					if(ed->num) free(c);
					SendMessage(hc,LB_DELETESTRING,i,0);
					SendMessage(hc,LB_INSERTSTRING,i,(long)buffer);
					SendMessage(hc,LB_SETCURSEL,i,0);
				} else {
					i=text_error-b;
					hc=GetDlgItem(win,3001);
					SendMessage(hc,EM_SETSEL,i,i);
					SetFocus(hc);
					free(b);
					break;
				}
				free(b);
				doc->t_modf=1;
			}
			break;
		case 3003:
			ed->num=0;
newstrings:
			SendDlgItemMessage(win,3000,LB_RESETCONTENT,0,0);
			goto updstrings;
		case 3004:
			ed->num=1;
			goto newstrings;
		}
		break;
	}
	return FALSE;
}
int Handlescroll(HWND win,int wparam,int sc,int page,int scdir,int size,int size2)
{
	SCROLLINFO si;
	int i=sc;
	switch(wparam&65535) {
	case SB_BOTTOM:
		i=size-page;
		break;
	case SB_TOP:
		i=0;
		break;
	case SB_LINEDOWN:
		i++;
		break;
	case SB_LINEUP:
		i--;
		break;
	case SB_PAGEDOWN:
		i+=page;
		break;
	case SB_PAGEUP:
		i-=page;
		break;
	case SB_THUMBPOSITION:
	case SB_THUMBTRACK:
		i=wparam>>16;
		break;
	}
	if(i>size-page) i=size-page;
	if(i<0) i=0;
	si.cbSize=sizeof(si);
	si.fMask=SIF_POS;
	si.nPos=i;
	SetScrollInfo(win,scdir,&si,1);
	if(scdir==SB_VERT) ScrollWindowEx(win,0,(sc-i)*size2,0,0,0,0,SW_INVALIDATE|SW_ERASE);
	else ScrollWindowEx(win,(sc-i)*size2,0,0,0,0,0,SW_INVALIDATE|SW_ERASE);
	return i;
}
void Loadeditinst(HWND win,SAMPEDIT*ed)
{
	ZINST*zi;
	zi=ed->ew.doc->insts+ed->editinst;
	SetDlgItemInt(win,3014,(zi->multhi<<8)+zi->multlo,0);
	wsprintf(buffer,"%04X",(zi->ad<<8)+zi->sr);
	SetDlgItemText(win,3016,buffer);
	wsprintf(buffer,"%02X",zi->gain);
	SetDlgItemText(win,3018,buffer);
	SetDlgItemInt(win,3020,zi->samp,0);
}

char mus_min[3]={0,15,31};
char mus_max[3]={15,31,34};

BOOL CALLBACK musdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	int i,j,k;
	HWND hc;
	MUSEDIT*ed;
	FDOC*doc;
	SONG*s;
	SONGPART*sp;
	char*st;
	switch(msg) {
	case WM_INITDIALOG:
		SetWindowLong(win,DWL_USER,lparam);
		ed=(MUSEDIT*)lparam;
		ed->dlg=win;
		ed->sel_song=-1;
		if(!ed->ew.doc->m_loaded) Loadsongs(ed->ew.doc);
		hc=GetDlgItem(win,3000);
		j=ed->ew.doc->numsong[ed->ew.param];
		for(i=0;i<j;i++) {
			if(i<mus_min[ed->ew.param] || i>=mus_max[ed->ew.param]) st=buffer,wsprintf(buffer,"Song %d",i+1);
			else st=mus_str[i+2];
			SendMessage(hc,LB_ADDSTRING,0,(long)st);
		}
		break;
	case WM_COMMAND:
		switch(wparam) {
		case 3000|(LBN_SELCHANGE<<16):
			ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
			i=SendMessage((HWND)lparam,LB_GETCURSEL,0,0);
			doc=ed->ew.doc;
			j=ed->ew.param;
			while(j--) i+=doc->numsong[j];
			if(ed->sel_song!=i) {
				ed->sel_song=i;
songchg:
				hc=GetDlgItem(win,3009);
				SendMessage(hc,LB_RESETCONTENT,0,0);
				s=doc->songs[i];
				if(s) {
					ShowWindow(hc,SW_SHOW);
					for(j=0;j<s->numparts;j++) {
						wsprintf(buffer,"Part %d",j);
						SendMessage(hc,LB_ADDSTRING,0,(long)buffer);
					}
					SetDlgItemInt(win,3025,s->lopst,0);
					CheckDlgButton(win,3026,(s->flag&2)?BST_CHECKED:BST_UNCHECKED);
					ShowWindow(GetDlgItem(win,3025),(s->flag&2)?SW_SHOW:SW_HIDE);
				} else ShowWindow(hc,SW_HIDE);
				for(j=0;j<8;j++) EnableWindow(GetDlgItem(win,3001+j),(int)s);
			}
			break;
		case 3009|(LBN_DBLCLK<<16):
			ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
			i=SendMessage((HWND)lparam,LB_GETCURSEL,0,0);
			if(i==-1) break;
			if(!sndinit) Initsound();
			if(!sndinit) break;
//			EnterCriticalSection(&cs_song);
			Stopsong();
			playedpatt=i;
			sounddoc=ed->ew.doc;
			playedsong=ed->ew.doc->songs[ed->sel_song];
			Playpatt();
//			LeaveCriticalSection(&cs_song);
			break;
		case 3009|(LBN_SELCHANGE<<16):
			ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
			i=SendMessage((HWND)lparam,LB_GETCURSEL,0,0);
			if(i==-1) break;
			doc=ed->ew.doc;
			j=ed->sel_song;
			ed->init=1;
			for(k=0;k<8;k++) {
				wsprintf(buffer,"%04X",ed->ew.doc->songs[j]->tbl[i]->tbl[k]&65535);
				SetDlgItemText(win,3012+k,buffer);
			}
			ed->init=0;
			break;
		case 3001:
		case 3002:
		case 3003:
		case 3004:
		case 3005:
		case 3006:
		case 3007:
		case 3008:
			i=wparam-3001;
			ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
			j=SendDlgItemMessage(win,3009,LB_GETCURSEL,0,0);
			if(j==-1 || ed->sel_song==-1) break;
			Edittrack(ed->ew.doc,ed->ew.doc->songs[ed->sel_song]->tbl[j]->tbl[i]);
			break;
		case 3010:
			ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
			if(ed->sel_song==-1 || !ed->ew.doc->songs[ed->sel_song]) break;
			if(!sndinit) Initsound();
			if(sndinit) Playsong(ed->ew.doc,ed->sel_song);
			break;
		case 3011:
			if(!sndinit) {sounddoc=0;break;}
//			EnterCriticalSection(&cs_song);
			Stopsong();
//			LeaveCriticalSection(&cs_song);
			break;
		case 3012|(EN_CHANGE<<16):
		case 3013|(EN_CHANGE<<16):
		case 3014|(EN_CHANGE<<16):
		case 3015|(EN_CHANGE<<16):
		case 3016|(EN_CHANGE<<16):
		case 3017|(EN_CHANGE<<16):
		case 3018|(EN_CHANGE<<16):
		case 3019|(EN_CHANGE<<16):
			ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
			if(ed->init) break;
			GetWindowText((HWND)lparam,buffer,sizeof(buffer));
			k=strtol(buffer,0,16);
			i=wparam-(3012|(EN_CHANGE<<16));
			j=SendDlgItemMessage(win,3009,LB_GETCURSEL,0,0);
			if(j==-1 || ed->sel_song==-1) break;
			s=ed->ew.doc->songs[ed->sel_song];
			if(!s) break;
			s->tbl[j]->tbl[i]=k;
			ed->ew.doc->m_modf=1;
			break;
		case 3020:
			ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
			j=SendDlgItemMessage(win,3009,LB_GETCURSEL,0,0);
			if(j==-1 || ed->sel_song==-1) break;
			s=ed->ew.doc->songs[ed->sel_song];
			if(!s) break;
			ed->ew.doc->sp_mark=s->tbl[j];
			break;
		case 3021:
			ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
			i=ed->sel_song;
			j=SendDlgItemMessage(win,3009,LB_GETCURSEL,0,0);
			if(i==-1) break;
			doc=ed->ew.doc;
			s=doc->songs[i];
			if(!s) break;
			s->numparts++;
			s->tbl=realloc(s->tbl,s->numparts<<2);
			if(j!=-1) MoveMemory(s->tbl+j+1,s->tbl+j,s->numparts-j<<2);
			else j=s->numparts-1;
			(s->tbl[j]=doc->sp_mark)->inst++;
			ed->ew.doc->m_modf=1;
			goto songchg;
		case 3022:
			ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
			i=ed->sel_song;
			if(i==-1) break;
			doc=ed->ew.doc;
			s=doc->songs[i];
			if(!s) break;
			s->numparts++;
			s->tbl=realloc(s->tbl,s->numparts<<2);
			sp=s->tbl[s->numparts-1]=malloc(sizeof(SONGPART));
			for(k=0;k<8;k++) sp->tbl[k]=-1;
			sp->flag=s->flag&1;
			sp->inst=1;
			ed->ew.doc->m_modf=1;
			goto songchg;
		case 3023:
			ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
			i=ed->sel_song;
			j=SendDlgItemMessage(win,3009,LB_GETCURSEL,0,0);
			if(j==-1 || i==-1) break;
			doc=ed->ew.doc;
			s=doc->songs[i];
			if(!s) break;
			s->numparts--;
			sp=s->tbl[j];
			sp->inst--;
			if(!sp->inst) free(sp);
			sp=CopyMemory(s->tbl+j,s->tbl+j+1,s->numparts-j<<2);
			ed->ew.doc->m_modf=1;
			goto songchg;
		case 3024:
			ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
			j=SendDlgItemMessage(win,3009,LB_GETCURSEL,0,0);
			i=ed->sel_song;
			if(i==-1) break;
			s=ed->ew.doc->songs[i];
			if(!s) break;
			NewSR(ed->ew.doc,((j==-1)?(s->flag&1):(s->tbl[j]->flag&1))?0:ed->ew.param+1);
			break;
		case 3025|(EN_CHANGE<<16):
			ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
			if(ed->init) break;
			i=ed->sel_song;
			if(i==-1) break;
			s=ed->ew.doc->songs[i];
			if(s) s->lopst=GetDlgItemInt(win,3025,0,0);
			ed->ew.doc->m_modf=1;
			break;
		case 3026:
			ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
			i=ed->sel_song;
			if(i==-1) break;
			s=ed->ew.doc->songs[i];
			if(s) {
				hc=GetDlgItem(win,3025);
				if((IsDlgButtonChecked(win,3026)==BST_CHECKED)) {
					s->flag|=2;
					ShowWindow(hc,SW_SHOW);
				} else {
					s->flag&=-3;
					ShowWindow(hc,SW_HIDE);
				}
				ed->ew.doc->m_modf=1;
			}
			break;
		case 3027:
			ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
			i=ed->sel_song;
			if(i==-1) break;
			doc=ed->ew.doc;
			if(doc->songs[i]) { MessageBox(framewnd,"Please delete the existing song first.","Bad error happened",MB_OK); break; }
			s=doc->songs[i]=malloc(sizeof(SONG));
			s->flag=0;
			s->inst=1;
			s->numparts=0;
			s->tbl=0;
			goto updsongs;
		case 3028:
			ed=(MUSEDIT*)GetWindowLong(win,DWL_USER);
			i=ed->sel_song;
			if(i==-1) break;
			doc=ed->ew.doc;
			s=doc->songs[i];
			if(s==playedsong) {
				if(sndinit) {
//					EnterCriticalSection(&cs_song);
					Stopsong();
//					LeaveCriticalSection(&cs_song);
				} else sounddoc=0;
			}
			doc->songs[i]=0;
			if(s) {
				s->inst--;
				if(!s->inst) {
					for(i=0;i<s->numparts;i++) {
						sp=s->tbl[i];
						sp->inst--;
						if(!sp->inst) free(sp);
					}
					free(s);
				}
updsongs:
				ed->ew.doc->m_modf=1;
				ed->sel_song=-1;
				musdlgproc(win,WM_COMMAND,3000|(LBN_SELCHANGE<<16),(long)GetDlgItem(win,3000));
			} else MessageBox(framewnd,"There is no song","Bad error happened",MB_OK);
			break;
		}
	}
	return FALSE;
}

const static char* chest_str[]={
	"Swd&&Sh1",
	"Sword 2",
	"Sword 3",
	"Sword 4",
	"Shield 1",
	"Shield 2",
	"Shield 3",
	"Firerod",
	"Icerod",
	"Hammer",
	"Hookshot",
	"Bow",
	"Boomerang",
	"Powder",
	"Bee",
	"Bombos",
	"Ether",
	"Quake",
	"Lamp",
	"Shovel",
	"Flute",
	"Red cane",
	"Bottle",
	"Heart piece",
	"Blue cane",
	"Cape",
	"Mirror",
	"Power glove",
	"Titan glove",
	"Wordbook",
	"Flippers",
	"Pearl",
	"Crystal",
	"Net",
	"Armor 2",
	"Armor 3",
	"Key",
	"Compass",
	"LiarHeart",
	"Bomb",
	"3 bombs",
	"Mushroom",
	"Red boom.",
	"Red pot",
	"Green pot",
	"Blue pot",
	"Red pot",
	"Green pot",
	"Blue pot",
	"10 bombs",
	"Big key",
	"Map",
	"1 rupee",
	"5 rupees",
	"20 rupees",
	"Pendant 1",
	"Pendant 2",
	"Pendant 3",
	"Bow&&Arrows",
	"Bow&&S.Arr",
	"Bee",
	"Fairy",
	"MuteHeart",
	"HeartCont.",
	"100 rupees",
	"50 rupees",
	"Heart",
	"Arrow",
	"10 arrows",
	"Magic",
	"300 rupees",
	"20 rupees",
	"Bee",
	"Sword 1",
	"Flute",
	"Boots",
	"No alternate"
};

char* sec_str[]={
	"Hole",
	"Warp",
	"Staircase",
	"Bombable",
	"Switch",
	"8A",
	"8C",
	"8E"
};

char* Getsecretstring(unsigned char*rom,int i)
{
	int a;
	if(i>=128) return sec_str[(i&15)>>1];
	else if(i==4) return "Random";
	else if(i==0) a=0;
	else a=rom[0x301f3+i];
	return a?sprname[a]:"Nothing";
}

void Dungselectchg(DUNGEDIT*ed,HWND hc,int f)
{
	RECT rc;
	unsigned char*rom=ed->ew.doc->rom;
	int i,j,k,l;
	static char *dir_str[4]={"Up","Down","Left","Right"};
	if(f) ed->ischest=0;
	if(!ed->selobj) {if(f) buffer[0]=0;}
	else if(ed->selchk==9) {
		dm_x=(*(short*)(ed->tbuf+ed->selobj))>>1;
		if(f) wsprintf(buffer,"Torch\nX: %02X\nY: %02X\nBG%d\nP: %d",dm_x&63,(dm_x>>6)&63,((dm_x>>12)&1)+1,(dm_x>>13)&7);
	} else if(ed->selchk==8) {
		dm_x=(*(short*)(rom+ed->selobj+2))>>1;
		if(f) wsprintf(buffer,"Block\nX: %02X\nY: %02X\nBG%d",dm_x&63,(dm_x>>6)&63,(dm_x>>12)+1);
	} else if(ed->selchk==7) {
		dm_x=*(short*)(ed->sbuf+ed->selobj-2)>>1;
		dm_k=ed->sbuf[ed->selobj];
		cur_sec=Getsecretstring(rom,dm_k);
		if(f) wsprintf(buffer,"Item %02X\nX: %02X\nY: %02X\nBG%d",dm_k,dm_x&63,(dm_x>>6)&63,(dm_x>>12)+1);
	} else if(ed->selchk==6) {
		dm_x=((ed->ebuf[ed->selobj+1]&31)<<1)+((ed->ebuf[ed->selobj]&31)<<7);
		dm_dl=ed->ebuf[ed->selobj]>>7;
		dm_l=((ed->ebuf[ed->selobj]&0x60)>>2)|((ed->ebuf[ed->selobj+1]&0xe0)>>5);
		dm_k=ed->ebuf[ed->selobj+2]+(((dm_l&7)==7)?256:0);
		if(f) wsprintf(buffer,"Spr %02X\nX: %02X\nY: %02X\nBG%d\nP: %02X",dm_k,dm_x&63,dm_x>>6,dm_dl+1,dm_l);
	} else if(ed->selchk&1) {
		getdoor(ed->buf+ed->selobj,rom);
		if(f) wsprintf(buffer,"Dir: %s\nType: %d\nPos: %d\n",dir_str[dm_k],dm_l,dm_dl);
	} else {
		getobj(ed->buf+ed->selobj);
		if(f) {
			if(dm_k>=0xf8 && dm_k<0x100) {
				wsprintf(buffer,"Obj: %03X:%X\nX: %02X\nY: %02X",dm_k,dm_l,dm_x&0x3f,dm_x>>6);
				if((dm_k==0xf9 && dm_l==9) || (dm_k==0xfb && dm_l==1)) {
					for(k=0;k<ed->chestnum;k++) if(ed->selobj==ed->chestloc[k]) break;
					for(l=0;l<0x1f8;l+=3) {
						if((*(short*)(rom+0xe96e+l)&0x7fff)==ed->mapnum) {
							k--;
							if(k<0) {
								k=rom[0xe970+l];
								i=rom[0x3b528+k];
								if(i==255) i=76;
								wsprintf(buffer+21,(rom[0xe96f+l]&128)?"\n%d:%s\n(%s)\n(Big)":"\n%d:%s\n(%s)",k,chest_str[k],chest_str[i]);
								ed->ischest=1;
								break;
							}
						}
					}
				}
			}
			else wsprintf(buffer,"Obj: %03X\nX: %02X\nY: %02X",dm_k,dm_x&0x3f,dm_x>>6);
			if(dm_k<0xf8) wsprintf(buffer+20,"\nSize: %02X",dm_l);
		}
	}
	if(f) SetDlgItemText(ed->dlg,3013,buffer);
	if(!ed->selobj) return;
	k=ed->mapscrollh;
	l=ed->mapscrollv;
	i=((dm_x&0x3f)<<3)-(k<<5);
	j=(((dm_x>>6)&0x3f)<<3)-(l<<5);
	if(f && ed->selobj) {
		if(i<0) k+=i>>5;
		if(j<0) l+=j>>5;
		if((i+32>>5)>=ed->mappageh) k+=(i+32>>5)-ed->mappageh;
		if((j+32>>5)>=ed->mappagev) l+=(j+32>>5)-ed->mappagev;
		if(k!=ed->mapscrollh) SendMessage(hc,WM_HSCROLL,SB_THUMBPOSITION+(k<<16),0);
		if(l!=ed->mapscrollv) SendMessage(hc,WM_VSCROLL,SB_THUMBPOSITION+(l<<16),0);
	}
	rc.left=((dm_x&0x3f)<<3);
	rc.top=(((dm_x>>6)&0x3f)<<3);
	Getdungobjsize(ed->selchk,&rc,0,0,0);
	ed->selrect=rc;
	rc.left-=k<<5;
	rc.top-=l<<5;
	rc.right-=k<<5;
	rc.bottom-=l<<5;
	ed->objt=dm_k;
	ed->objl=dm_l;
	InvalidateRect(hc,&rc,0);
}

//Drawblock#********************************

void Drawblock(OVEREDIT *ed, int x, int y, int n, int t)
{
	register char *b1, *b2, *b3, *b4;
	
	int a,
		b,
		c,
		d;

	unsigned e;
	
	const static char f[14] = {1,1,0,0,0,0,0,0,1,1,1,0,1,1};

	int col;
	int mask,tmask;
	
	b2 = (drawbuf + 992 + x - (y << 5));
	
	d = n & 0x3ff;
	
	*(char*)&col = *(((char*)&col) + 1) = *(((char*)&col) + 2) = *(((char*)&col)+3) = ((n & 0x1c00) >> 6);
	
	if((t & 24) == 24)
	{
		b3 = ed->ew.doc->blks[225].buf;
		
		if(!b3) 
			goto noblock;
		
		b1 = b3 + (n << 6);
		mask = 0xf0f0f0f;
		col = 0;
		b4 = b1 + 0xe000;
	} 
	else if(t & 16)
	{
		if(d >= 0x180)
			goto noblock;
		
		b3 = ed->ew.doc->blks[bg3blkofs[d>>7]].buf;
		
		if(!b3) 
			goto noblock;
		
		b1 = b3 + ((d & 0x7f) << 6);
		mask = 0x3030303;
		col >>= 2;
	
		if(n & 0x4000)
			b1 += 0x4000;

		b4 = b1 + 0x2000;
	} 
	else if(t & 8)
	{
		if(d >= 0x100)
			goto noblock;
		
		b3 = ed->ew.doc->blks[224].buf;
		
		if(!b3) 
			goto noblock;
		
		b1 = b3+(d<<6);
		mask = 0x3030303;
		col >>= 2;
		
		if(n & 0x4000) 
			b1 += 0x8000;
		
		b4 = b1 + 0x4000;
	} 
	else if(d >= 0x200)
	{
		if(d < 0x240)
			goto noblock;
		else if(d < 0x280) 
		{
			b3 = ed->ew.doc->blks[ed->blocksets[10]].buf;
			
			if(!b3) 
				goto noblock;
			
			b1 = b3 + ((d - 0x240) << 6);
			mask = 0xf0f0f0f;
		} 
		else if(d < 0x300)
		{
			b3 = ed->ew.doc->blks[0x79+(d-0x280>>6)].buf;
			
			if(!b3) 
				goto noblock;
			
			b1 = b3 + (((d - 0x280) & 63) << 6);
			mask = 0x7070707;
		} 
		else 
		{
			e = ed->blocksets[(d >> 6) - 1];
			b3 = ed->ew.doc->blks[e].buf;
			
			if(!b3) 
				goto noblock;
			
			b1 = b3 + ((d & 0x3f) << 6);
			
			if(e >= 0xc5 && e < 0xd3)
				mask = f[e - 0xc5] ? 0xf0f0f0f : 0x7070707;
			else 
				mask = 0x7070707;
		}

		if(n & 0x4000) 
			b1 += 0x2000;
		
		b4 = b1 + 0x1000;
	} 
	else if(ed->gfxtmp == 0xff) 
	{
		mask = -1;
		col = 0;
		b1 = ed->ew.doc->blks[223].buf + (n << 6);
	} 
	else 
	{
		if(ed->gfxtmp >= 0x20) 
			mask = palhalf[(n & 0x1c0) >> 6] ? 0xf0f0f0f : 0x7070707;
		else
			mask = (n & 0x100) ? 0x7070707 : 0xf0f0f0f;
	
		if(ed->anim != 3) 
		{
			if(ed->gfxtmp >= 0x20)
			{
				e = d - 0x1c0;
				
				if(e < 0x20u) 
					d = (ed->anim << 5) + e + 0x200;
			} 
			else 
			{
				e=d-0x1b0;
				if(e<0x20u) {
					d=(ed->anim<<4)+e+0x200;
					if(e>=0x10) d+=0x30;
				}
			}
		}
		
		if(n & 0xff0000) 
			b3 = ed->ew.doc->blks[(n >> 16) - 1].buf;
		else 
			b3 = ed->ew.doc->blks[ed->blocksets[d >> 6]].buf;
		
		if(!b3)
		{
noblock:
			if(t & 1) 
				return;

			for(a = 0; a < 8; a++)
			{
				*(int*)b2 = 0;
				((int*)b2)[1] = 0;
			
				b2 -= 32;
			}
			return;
		}
	
		b1 = b3 + ((d & 0x3f) << 6);
	
		if(n & 0x4000) 
			b1 += 0x2000;

		b4 = b1 + 0x1000;
	}
	
	if(t & 4) 
		col += 0x80808080;
	
	switch(t & 3)
	{
	case 0:
		switch(n & 0x8000) 
		{
		case 0:
			
			for(a = 0; a < 8; a++) 
			{
				*(int*)b2 = ((*(int*)b1) & mask) + col;
				((int*)b2)[1]=((((int*)b1)[1])&mask)+col;
				
				b2 -= 32;
				b1 += 8;
			}
			
			break;
		
		case 0x8000:
			
			b2 -= 224;
			
			for(a = 0; a < 8; a++)
			{
				*(int*)b2 = ((*(int*)b1) & mask) + col;
				((int*)b2)[1] = ((((int*)b1)[1]) & mask) + col;
				b2 += 32;
				b1 += 8;
			}
	
			break;
		}

		break;
	
	case 1:
	
		switch(n & 0x8000)
		{
		case 0:
		
			for(a = 0; a < 8; a++) 
			{
				b = ((int*)b4)[0];
				c = ((int*)b4)[1];
				
				((int*)b2)[0] &= b;
				((int*)b2)[1] &= c;
				((int*)b2)[0] |= (((*(int*)b1) & mask) + col) & ~b;
				((int*)b2)[1] |= (((*(int*)(b1+4)) & mask) + col) & ~c;
			
				b2 -= 32;
				b1 += 8;
				b4 += 8;
			}
	
			break;
		
		case 0x8000:
		
			b2 -= 224;
			
			for(a = 0; a < 8; a++)
			{
				b = ((int*)b4)[0];
				c = ((int*)b4)[1];
			
				((int*)b2)[0] &= b;
				((int*)b2)[1] &= c;
				((int*)b2)[0] |= (((*(int*)b1)&mask)+col)&~b;
				((int*)b2)[1] |= (((*(int*)(b1+4))&mask)+col)&~c;
				
				b2 += 32;
				b1 += 8;
				b4 += 8;
			}
		
			break;
		}
		
		break;
	
	case 2:
	
		switch(n & 0x8000) 
		{
		case 0:
		
			tmask = 0xFF00FF;
			
			for(a = 0; a < 8; a++)
			{
				*(int*)b2 = (((*(int*)b1) & mask) + col) & tmask;
				((int*)b2)[1] = (((((int*)b1)[1]) & mask) + col) & tmask;
			
				b2 -= 32;
				b1 += 8;
				tmask ^= -1;
			}
	
			break;
		
		case 0x8000:
			
			b2 -= 224;
			tmask = 0xFF00FF00;
		
			for(a = 0; a < 8; a++)
			{
				*(int*)b2 = (((*(int*)b1) & mask) + col) & tmask;
				((int*)b2)[1] = (((((int*)b1)[1]) & mask) + col) & tmask;
				
				b2 += 32;
				b1 += 8;
				
				tmask ^= -1;
			}

			break;
		}
		
		break;
	
	case 3:
	
		switch(n & 0x8000) 
		{
		case 0:
		
			tmask = 0xFF00FF;
		
			for(a = 0; a < 8; a++)
			{
				b = ((int*) b4)[0] | ~tmask;
				c = ((int*) b4)[1] | ~tmask;
			
				((int*) b2)[0] &= b;
				((int*) b2)[1] &= c;

				((int*) b2)[0] |= (((*(int*)b1) & mask)+col) & ~b & tmask;
				((int*) b2)[1] |= (((*(int*)(b1+4)) & mask) + col) & ~c & tmask;
	
				b2 -= 32;
				b1 += 8;
				b4 += 8;

				tmask ^= -1;
			}
		
			break;
	
		case 0x8000:
			
			tmask = 0xFF00FF00;
			b2 -= 224;
			
			for(a = 0; a < 8; a++)
			{
				b = ((int*) b4)[0] | ~tmask;
				c = ((int*) b4)[1] | ~tmask;
				
				((int*) b2)[0] &= b;
				((int*) b2)[1] &= c;
			
				((int*) b2)[0] |= (((*(int*) b1) & mask) + col) & ~b & tmask;
				((int*) b2)[1] |= (((*(int*) (b1 + 4)) & mask) + col) & ~c & tmask;
				
				b2 += 32;
				b1 += 8;
				b4 += 8;

				tmask ^= -1;
			}
		
			break;
		}
	
		break;
	}
}

//Drawblock*********************************

int gbtnt=0;
void Paintblocks(RECT*rc,HDC hdc,int x,int y,DUNGEDIT*ed)
{
	int a,b,c,d;
	if(gbtnt) {
		c=0,d=0;
		if(x<rc->left) c=rc->left-x,x=rc->left;
		if(y<rc->top) d=rc->top-y,y=rc->top;
		a=32-c;
		b=32-d;
		if(x+a>rc->right) a=rc->right-x;
		if(y+b>rc->bottom) b=rc->bottom-y;
		if(a<1 || b<1 || c>31 || d>31) return;
		SetDIBitsToDevice(hdc,x,y,a,b,0,0,0,32,drawbuf+c+(d+b-32<<5),(BITMAPINFO*)&(ed->bmih),DIB_RGB_COLORS);
	} else SetDIBitsToDevice(hdc,x,y,32,32,0,0,0,32,drawbuf,(BITMAPINFO*)&(ed->bmih),ed->hpal?DIB_PAL_COLORS:DIB_RGB_COLORS);
}

void Paintdungeon(DUNGEDIT*ed,HDC hdc,RECT*rc,int x,int y,int k,int l,int n,int o,unsigned short*buf)
{
	int i,j,m,p,q,r,s,t,u,v;
	HBRUSH oldobj;
	if((!(ed->disp&3)) || ((!(ed->disp&1)) && !(ed->layering>>5))) {
		oldobj=SelectObject(hdc,black_brush);
		v=1;
	} else v=0;
	for(j=y;j<l;j+=32) {
		i=x;
		for(;i<k;i+=32) {
			if(v)
				Rectangle(hdc,i,j,i+32,j+32);
			else {
				m=(i+n>>3)+(j+o<<3);
			for(p=0;p<4;p++) for(q=0;q<256;q+=64) {
				r=0;
				r=p<<3;
				s=q>>3;
				t=m+p+q;
				u=0;
				if(ed->layering&2) Drawblock((OVEREDIT*)ed,r,s,buf[t+0x2000],8),u=1;
				switch(ed->layering>>5) {
				case 0:
					if(ed->disp&1) Drawblock((OVEREDIT*)ed,r,s,buf[t],u);
					break;
				case 1: case 5:
				case 6:
					if(ed->disp&2) Drawblock((OVEREDIT*)ed,r,s,buf[t+0x1000],u),u=1;
					if(ed->disp&1) Drawblock((OVEREDIT*)ed,r,s,buf[t],u);
					break;
				case 2:
					if(ed->disp&2) Drawblock((OVEREDIT*)ed,r,s,buf[t+0x1000],u+2),u=1;
					if(ed->disp&1) Drawblock((OVEREDIT*)ed,r,s,buf[t],u);
					break;
				case 3:
					if(ed->disp&1) Drawblock((OVEREDIT*)ed,r,s,buf[t],u),u=1;
					if(ed->disp&2) Drawblock((OVEREDIT*)ed,r,s,buf[t+0x1000],u);
					break;
				case 4: case 7:
					if(ed->disp&1) Drawblock((OVEREDIT*)ed,r,s,buf[t],u),u=1;
					if(ed->disp&2) Drawblock((OVEREDIT*)ed,r,s,buf[t+0x1000],u+2);
					break;
				}
				if(ed->layering&4) Drawblock((OVEREDIT*)ed,r,s,buf[t+0x2000],17);
			}
			Paintblocks(rc,hdc,i,j,ed);
			}
		}
	}
	if(v) SelectObject(hdc,oldobj);
}

void Updateobjdisplay(CHOOSEDUNG*ed,int num)
{
	int i=num+(ed->scroll<<2),j,k,l,m,n,o,p;
	short objbuf[8192];
	unsigned char obj[6];
	RECT rc;
	for(j=0;j<8192;j++) objbuf[j]=0x1e9;
	if(i>=0x260) return;
	else if(i>=0x1b8) {
		obj[0]=0xf0;
		obj[1]=0xff;
		obj[2]=(i-0x1b8)/42;
		obj[3]=((i-0x1b8)%42)<<1;
		obj[4]=0xff;
		obj[5]=0xff;
		getdoor(obj+2,ed->ed->ew.doc->rom);
		rc.left=(dm_x&0x3f)<<3;
		rc.top=(dm_x&0xfc0)>>3;
	} else {
		if(i<0x40) {
			obj[0]=0xfc;
			obj[1]=0;
			obj[2]=i;
		} else if(i<0x138) {
			obj[0]=0;
			obj[1]=0;
			switch(obj3_t[i-0x40]&15) {
			case 0: case 2:
				obj[1]=1;
				break;	
			}
			obj[2]=i-0x40;
		} else {
			obj[0]=i&3;
			obj[1]=(i-0x138>>2)&3;
			obj[2]=0xf8+((i-0x138>>4)&7);
		}
		obj[3]=255;
		obj[4]=255;
		getobj(obj);
		rc.left=0;
		rc.top=0;
	}
	Getdungobjsize(ed->ed->selchk,&rc,0,0,1);
	if(i<0x1b8) {
		if(rc.top<0) j=-rc.top>>3; else j=0;
		if(rc.left<0) k=-rc.left>>3; else k=0;
		if(i<0x40) {
			obj[0]|=k>>4;
			obj[1]=(k<<4)|(j>>2);
			obj[2]|=k<<6;
		} else {
			obj[0]|=k<<2;
			obj[1]|=j<<2;
		}
		rc.right+=k<<3;
		rc.bottom+=j<<3;
		rc.left+=k<<3;
		rc.top+=j<<3;
	}
	dm_buf=objbuf;
	Drawmap(ed->ed->ew.doc->rom,objbuf,obj,ed->ed);
	Paintdungeon(ed->ed,objdc,&rc,rc.left,rc.top,rc.right,rc.bottom,0,0,objbuf);
	rc.right-=rc.left;
	rc.bottom-=rc.top;
	n=rc.right;
	if(rc.bottom>n) n=rc.bottom;
	n++;
	j=ed->w>>2;
	k=ed->h>>2;
	l=(rc.right)*j/n;
	m=(rc.bottom)*k/n;
	o=((((i&3)<<1)+1)*ed->w>>3);
	p=(((((i>>2)&3)<<1)+1)*ed->h>>3);
	StretchBlt(ed->bufdc,o-(l>>1),p-(m>>1),l,m,objdc,rc.left,rc.top,rc.right,rc.bottom,SRCCOPY);
	if(i<0x40) l=i+0x100;
	else if(i<0x138) l=i-0x40;
	else if(i<0x1b8) l=i+0xe48;
	else l=i-0x1b8;
	wsprintf(buffer,"%03X",l);
	SetTextColor(ed->bufdc,0);
	TextOut(ed->bufdc,o+1,p+1,buffer,3);
	SetTextColor(ed->bufdc,0xffbf3f);
	TextOut(ed->bufdc,o,p,buffer,3);
}
void Getdungselrect(int i,RECT*rc,CHOOSEDUNG*ed)
{
	rc->left=(i&3)*ed->w>>2;
	rc->top=(i>>2)*ed->h>>2;
	rc->right=rc->left+(ed->w>>2);
	rc->bottom=rc->top+(ed->h>>2);
}
void Setpalette(HWND win,HPALETTE pal)
{
	HPALETTE oldpal,hdc;
	hdc=GetDC(win);
	oldpal=SelectPalette(hdc,pal,0);
	RealizePalette(hdc);
	SelectPalette(hdc,oldpal,1);
	ReleaseDC(win,hdc);
}

unsigned short dp_x[]={
	2,98,52,
	3,17,0x863c,0x1023,18,
	6,56,0x1023,34,0x1023,56,0x8590,34,
	4,0x83d8,49,0x872a,51,0x1023,51,
	4,0x875a,51,0x1023,51,0x83d8,49,
	16,0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,
	0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,
	1,0x9488,97,
	4,0x96dc,0x10cc,0x97f6,0x10cc,0x9914,0x10cc,0x9a2a,0x10cc,
	6,0x9bf2,70,81,0x1064,0x1026,0x1062,85,
	4,18,0x1023,18,68,
	2,0x22d6,0x1023,
	1,0x80e0,0x1024,
	3,34,18,50,
	4,34,34,34,34,
	2,51,51,
	1,0x9b4a,230
};
unsigned short dp_tab[]={
	68,68,68,68,68,68,68,68,68,68,68,68,68,68,68,68,
	67,67,67,67,52,52,52,52,34,34,34,34,68,50,34,34,
	34,50,0x1054,52,68,68,50,34,0x1054,68,0x1024,50,54,68,68,68,
	68,68,68,68,34,0x1024,0x1024,74,52,52,52,52,0xf000,51,54,113,
	34,66,66,66,66,66,66,34,34,81,81,81,81,81,81,81,
	81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,81,
	81,51,19,19,19,19,19,19,19,19,19,19,19,19,19,22,
	22,0,0,68,17,0,68,68,50,66,52,52,34,66,34,19,
	19,19,19,19,19,19,19,83,51,0x1024,0x1024,34,51,68,68,68,
	17,54,54,34,0,0x1024,0x1024,0,0,0,0,54,54,53,34,19,
	34,0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,34,34,49,17,17,97,97,0,0,
	68,17,0,68,68,66,67,67,34,17,17,34,17,34,0,66,
	66,67,67,67,67,0x1063,0x1063,66,0xf04e,34,49,17,17,17,17,17,
	0x1024,0x1024,34,34,68,34,34,0,0,0,0,0,0,0,0,0,
	17,17,17,17,0xf003,17,17,17,17,17,17,17,17,17,17,17,
	17,17,68,19,19,66,66,66,34,34,68,34,34,34,0,0,
	17,0xf008,17,49,0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,0,0,0xf010,0xf017,0,
	0,0x1024,0x1024,0,0,0,0x1024,17,0x1024,0x1024,0x1024,0xf01e,0x1035,0x1044,34,0x1024,
	0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,0x1024,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,
	0x1034,0x1054,0x1074,17,17,17,17,17,17,17,17,17,17,0xf02f,17,17,
	34,34,33,34,52,0x108a,34,0xf02f,34,34,34,68,68,68,68,68,
	68,68,34,34,34,34,68,68,68,68,0xf032,34,0xf052,0xf05a,0xf03b,34,
	34,52,52,68,35,35,54,54,35,35,0x1064,0x1064,70,70,34,34,
	34,34,34,34,34,34,34,68,68,34,34,56,0x1086,54,52,34,
	34,34,34,34,0xf043,0x1033,34,34,34,34,0x1024,0x1053,70,54,34,34,
	0xf057,0xf057,0xf048,34,34,34,68,52,52,67,67,68,52,52,67,67,
	72,0x2110,0x108a,0xf04b,0x2110,34,56,56,72,52,68,0x1024,34,34,34,0
};
void Updateblk8sel(BLOCKSEL8*ed,int num)
{
	int i=num+(ed->scroll<<4);
	int j,k,l,m;
	RECT rc;
	rc.left=0;
	rc.right=8;
	rc.top=0;
	rc.bottom=8;
	if(wver) i^=0x8000;
	Drawblock(ed->ed,0,24,i^ed->flags,((i&0x200)>>7)+ed->dfl);
	j=(i&15)*ed->w>>4;
	k=(i&240)*ed->h>>8;
	l=(((i&15)+1)*ed->w>>4)-j;
	m=(((i&240)+16)*ed->h>>8)-k;
	StretchDIBits(ed->bufdc,j,k,l,m,0,0,wver?33:8,wver?-8:8,drawbuf,(BITMAPINFO*)&(ed->ed->bmih),ed->ed->hpal?DIB_PAL_COLORS:DIB_RGB_COLORS,SRCCOPY);
	l--;
	m--;
//	MoveToEx(ed->bufdc,j+l,k,0);
//	LineTo(ed->bufdc,j+l,k+m);
//	LineTo(ed->bufdc,j-1,k+m);
}
DUNGEDIT*dunged;

void InitBlksel8(HWND hc,BLOCKSEL8*bs,HPALETTE hpal,HDC hdc)
{
	int i;
	RECT rc;
	HPALETTE oldpal;
	GetClientRect(hc,&rc);
	bs->sel=0;
	bs->scroll=0;
	bs->flags=0;
	bs->dfl=0;
	bs->w=rc.right;
	bs->h=rc.bottom;
	bs->bufdc=CreateCompatibleDC(hdc);
	bs->bufbmp=CreateCompatibleBitmap(hdc,rc.right,rc.bottom);
	SelectObject(bs->bufdc,bs->bufbmp);
	SelectObject(bs->bufdc,white_pen);
	SelectObject(bs->bufdc,black_brush);
	oldpal=SelectPalette(objdc,hpal,1);
	SelectPalette(bs->bufdc,hpal,1);
	Rectangle(bs->bufdc,0,0,bs->w,bs->h);
	for(i=0;i<256;i++) Updateblk8sel(bs,i);
	SelectPalette(objdc,oldpal,1);
	SelectPalette(bs->bufdc,oldpal,1);
	SetWindowLong(hc,GWL_USERDATA,(int)bs);
	Updatesize(hc);
}
int CALLBACK dpceditproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	unsigned short*rdbuf;
	int i,j,k,l,m,n,o,p,q,r;
	unsigned short*b;
	RECT rc;
	switch(msg) {
	case WM_LBUTTONUP:
		if(dpce->flag&1) {
			dpce->flag&=-2;
			ReleaseCapture();
		}
		break;
	case WM_LBUTTONDOWN:
		SetCapture(win);
		dpce->flag|=1;
	case WM_MOUSEMOVE:
		if(dpce->flag&1) {
	case WM_RBUTTONDOWN:
			k=dpce->sel;
			l=dpce->dp_w[k];
			m=dpce->dp_h[k];
			rc.left=(dpce->w>>1)-(l<<2);
			rc.top=(dpce->h>>1)-((m&127)<<2);
			i=((short)lparam-rc.left)>>3;
			j=((lparam>>16)-rc.top)>>3;
			if(i<0 || j<0 || i>=l || j>=(m&127)) break;
			b=(unsigned short*)(dpce->buf+dpce->dp_st[k]);
			if(m&128)
				b+=i+j*l;
			else b+=j+i*m;
			if(msg==WM_RBUTTONDOWN) {SendMessage(GetParent(win),4000,*b,0);break;}
			*b=dpce->bs.sel|dpce->bs.flags;
			rc.left+=i<<3;
			rc.top+=j<<3;
			rc.right=rc.left+8;
			rc.bottom=rc.top+8;
			InvalidateRect(win,&rc,0);
		}
		break;
	case WM_PAINT:
		hdc=BeginPaint(win,&ps);
		i=dpce->sel;
		j=dpce->dp_w[i];
		k=dpce->dp_h[i];
		l=dpce->dp_st[i];
		r=k;
		k&=127;
		rdbuf=(unsigned short*)(dpce->buf+l);
		l=0;
		rc.left=(dpce->w>>1)-(j<<2);
		rc.right=rc.left+(j<<3);
		rc.top=(dpce->h>>1)-(k<<2);
		rc.bottom=rc.right+(k<<3);
		if(r&128) {
		for(n=0;n<k;n+=4) {
			for(m=0;m<j;m+=4) {
				o=4;
				p=4;
				if(m>j-4) o=j&3;
				if(n>k-4) p=k&3;
				for(q=0;q<p;q++) {
					for(i=0;i<o;i++) {
						Drawblock((OVEREDIT*)dunged,i<<3,q<<3,rdbuf[l+i],0);
					}
					l+=j;
				}
				l-=j*p-o;
				o<<=3;
				p<<=3;
				SetDIBitsToDevice(hdc,rc.left+(m<<3),rc.top+(n<<3),o,p,0,0,0,32,drawbuf+(32-p<<5),(BITMAPINFO*)&(dunged->bmih),dunged->hpal?DIB_PAL_COLORS:DIB_RGB_COLORS);
			}
			l+=3*j;
		}
		} else {
		for(m=0;m<j;m+=4) {
			for(n=0;n<k;n+=4) {
				o=4;
				p=4;
				if(m>j-4) o=j&3;
				if(n>k-4) p=k&3;
				for(i=0;i<o;i++) {
					for(q=0;q<p;q++) {
						Drawblock((OVEREDIT*)dunged,i<<3,q<<3,rdbuf[l+q],0);
					}
					l+=k;
				}
				l-=k*o-p;
				o<<=3;
				p<<=3;
				SetDIBitsToDevice(hdc,rc.left+(m<<3),rc.top+(n<<3),o,p,0,0,0,32,drawbuf+(32-p<<5),(BITMAPINFO*)&(dunged->bmih),dunged->hpal?DIB_PAL_COLORS:DIB_RGB_COLORS);
			}
			l+=3*k;
		}
		}
		EndPaint(win,&ps);
		break;
	default:
		return DefWindowProc(win,msg,wparam,lparam);
	}
	return 0;
}
int loadpiece(int k,int j,int m)
{
	int n,o;
	if(j<0x2000) {
		n=dpce->dp_w[k]=j&15;
		n*=o=(j>>4)&15;
		o|=(j&0x1000)>>5;
		dpce->dp_h[k]=o;
		dpce->dp_st[k]=m;
		m+=n<<1;
	} else if(j<0x4000) {
		n=dpce->dp_w[k]=j&63;
		n*=o=(j>>6)&63;
		o|=(j&0x1000)>>5;
		dpce->dp_h[k]=o;
		dpce->dp_st[k]=m;
		m+=n<<1;		
	}
	return m;
}

void Changeblk8sel(HWND win,BLOCKSEL8*ed)
{
	RECT rc2;
	int i=ed->sel-(ed->scroll<<4);
	if(i>=0 && i<256) {
		rc2.left=(i&15)*ed->w>>4;
		rc2.right=rc2.left+(ed->w>>4);
		rc2.top=(i&240)*ed->h>>8;
		rc2.bottom=rc2.top+(ed->h>>4);
		InvalidateRect(win,&rc2,0);
	}
}

void Upddpcbuttons(HWND win)
{
	EnableWindow(GetDlgItem(win,IDC_BUTTON1),dpce->sel);
	EnableWindow(GetDlgItem(win,IDC_BUTTON8),dpce->sel<dpce->num-1);
	if(!IsWindowEnabled(GetFocus())) SetFocus(GetDlgItem(win,IDOK));
}

int GetBTOfs(DPCEDIT*ed)
{
	if(ed->bs.sel<0x140) return ed->bs.sel+0x71659;
	else if(ed->bs.sel>=0x1c0) return ed->bs.sel+0x715d9;
	else return ed->bs.sel+0x70eea+((short*)(ed->bs.ed->ew.doc->rom+0x71000))[((DUNGEDIT*)(ed->bs.ed))->gfxnum];
}

BOOL CALLBACK dpcdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	HDC hdc;
	int i,j,k,l,m;
	short*b;
	HPALETTE oldpal;
	RECT rc;
	HWND hc;
	BLOCKSEL8*bs;
	switch(msg) {
	case WM_QUERYNEWPALETTE:
		Setpalette(win,dunged->hpal);
		return 1;
	case WM_PALETTECHANGED:
		InvalidateRect(GetDlgItem(win,IDC_CUSTOM1),0,0);
		InvalidateRect(GetDlgItem(win,IDC_CUSTOM2),0,0);
		break;
	case WM_INITDIALOG:
		dpce=malloc(sizeof(DPCEDIT));
		dpce->sel=0;
		dpce->flag=0;
		dpce->bs.ed=(OVEREDIT*)dunged;
		SendDlgItemMessage(win,IDC_BUTTON1,BM_SETIMAGE,IMAGE_BITMAP,(int)arrows_imgs[0]);
		SendDlgItemMessage(win,IDC_BUTTON8,BM_SETIMAGE,IMAGE_BITMAP,(int)arrows_imgs[1]);
		memcpy(dpce->buf,dunged->ew.doc->rom+0x1b52,0x342e);
		hdc=GetDC(win);
		InitBlksel8(GetDlgItem(win,IDC_CUSTOM1),&(dpce->bs),dunged->hpal,hdc);
		ReleaseDC(win,hdc);
		hdc=GetDlgItem(win,IDC_CUSTOM2);
		GetClientRect(hdc,&rc);
		dpce->w=rc.right;
		dpce->h=rc.bottom;
		dpce->obj=i=lparam;
		j=dp_tab[i];
		if(i<0x40)
		m=((unsigned short*)(dunged->ew.doc->rom+0x83f0))[i];
		else if(i<0x138) m=((unsigned short*)(dunged->ew.doc->rom+0x7f80))[i];
		else if(i<0x1b8) m=((unsigned short*)(dunged->ew.doc->rom+0x8280))[i];
		if((j&0xf000)==0xf000) {
			b=dp_x+(j&4095);
			l=dpce->num=*(b++);
			for(k=0;k<l;k++) {
again:
				j=*(b++);
				if(j&32768) { m=j&32767; goto again; }
				m=loadpiece(k,j,m);
			}
		} else {
			dpce->num=1;
			m=loadpiece(0,j,m);
		}
		Upddpcbuttons(win);
		wparam=0;
	case 4000:
		bs=&(dpce->bs);
		bs->flags=wparam&0xfc00;
		CheckDlgButton(win,IDC_CHECK1,(wparam&16384)?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(win,IDC_CHECK2,(wparam&32768)?BST_CHECKED:BST_UNCHECKED);
		CheckDlgButton(win,IDC_CHECK3,(wparam&8192)?BST_CHECKED:BST_UNCHECKED);
		SetDlgItemInt(win,IDC_EDIT2,wparam&1023,0);
		SetDlgItemInt(win,IDC_EDIT1,(wparam>>10)&7,0);
		break;
	case WM_DESTROY:
		DeleteDC(dpce->bs.bufdc);
		DeleteObject(dpce->bs.bufbmp);
		free(dpce);
		break;
	case WM_COMMAND:
		switch(wparam) 
		{
		case IDC_EDIT2|(EN_CHANGE<<16):
			bs=&(dpce->bs);
			i=GetDlgItemInt(win,IDC_EDIT2,0,0);
			if(i<0) i=0;
			if(i>0x3ff) i=0x3ff;
			j=bs->scroll<<4;
			if(i<j) j=i;
			if(i>=j+256) j=i-240;
			hc=GetDlgItem(win,IDC_CUSTOM1);
			SendMessage(hc,WM_VSCROLL,SB_THUMBPOSITION|((j>>4)<<16),SB_VERT);
			Changeblk8sel(hc,bs);
			bs->sel=i;
			if(i<0x200) {
				SetDlgItemInt(win,IDC_EDIT8,dunged->ew.doc->rom[GetBTOfs(dpce)],0);
				ShowWindow(GetDlgItem(win,IDC_EDIT8),SW_SHOW);
				ShowWindow(GetDlgItem(win,IDC_STATIC2),SW_SHOW);
			} else {
				ShowWindow(GetDlgItem(win,IDC_EDIT8),SW_HIDE);
				ShowWindow(GetDlgItem(win,IDC_STATIC2),SW_HIDE);
			}
			Changeblk8sel(hc,bs);
			break;

		case IDC_EDIT1|(EN_CHANGE<<16):
			bs=&(dpce->bs);
			bs->flags&=0xe000;
			bs->flags|=(GetDlgItemInt(win,IDC_EDIT1,0,0)&7)<<10;
			goto updflag;
			break;

		case IDC_EDIT3|(EN_CHANGE<<16):
			bs=&(dpce->bs);
			if(bs->sel<0x200)
			bs->ed->ew.doc->rom[GetBTOfs(dpce)]=GetDlgItemInt(win,IDC_EDIT3,0,0);
			break;
		case IDC_CHECK1:
			bs=&(dpce->bs);
			bs->flags&=0xbc00;
			if(IsDlgButtonChecked(win,IDC_CHECK1)) bs->flags|=0x4000;
			goto updflag;
		case IDC_CHECK2:
			bs=&(dpce->bs);
			bs->flags&=0x7c00;
			if(IsDlgButtonChecked(win,IDC_CHECK2)) bs->flags|=0x8000;
updflag:
			if((bs->flags&0xdc00)!=bs->oldflags) {
				bs->oldflags=bs->flags&0xdc00;
				oldpal=SelectPalette(objdc,dunged->hpal,1);
				SelectPalette(bs->bufdc,dunged->hpal,1);
				for(i=0;i<256;i++) Updateblk8sel(bs,i);
				SelectPalette(objdc,oldpal,1);
				SelectPalette(bs->bufdc,oldpal,1);
				InvalidateRect(GetDlgItem(win,IDC_CUSTOM1),0,0);
			}
			break;
		case IDC_CHECK3:
			bs=&(dpce->bs);
			bs->flags&=0xdc00;
			if(IsDlgButtonChecked(win,IDC_CHECK3)) bs->flags|=0x2000;
			break;
		case IDOK:
			memcpy(dunged->ew.doc->rom+0x1b52,dpce->buf,0x342e);
			dunged->ew.doc->modf=1;
			EndDialog(win,1);
			break;
		case IDCANCEL:
			EndDialog(win,0);
			break;
		case IDC_BUTTON1:
			dpce->sel--;
			InvalidateRect(GetDlgItem(win,IDC_CUSTOM2),0,1);
			Upddpcbuttons(win);
			break;
		case IDC_BUTTON8:
			dpce->sel++;
			InvalidateRect(GetDlgItem(win,IDC_CUSTOM2),0,1);
			Upddpcbuttons(win);
		}
	}
	return 0;
}
int CALLBACK dungselproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	CHOOSEDUNG*ed;
	PAINTSTRUCT ps;
	HDC hdc;
	HPALETTE oldpal;
	SCROLLINFO si;
	int i,j,k,l,m,n;
	RECT rc;
	switch(msg) {
	case WM_GETDLGCODE:
		return DLGC_WANTCHARS;
	case WM_KEYDOWN:
		if(wparam>=65 && wparam<71) { wparam-=55; goto digscr; }
		if(wparam>=48 && wparam<58) {
			wparam-=48;
digscr:
			ed=(CHOOSEDUNG*)GetWindowLong(win,GWL_USERDATA);
			ed->typednum=((ed->typednum<<4)|wparam)&0xfff;
			i=ed->typednum;
			if(!(ed->ed->selchk&1))
				if(i>=0xf80) i-=0xe48;
				else if(i>=0x140) break;
				else if(i>=0x100) i-=0x100;
				else if(i>=0xf8) break;
				else i+=0x40;
				else i+=0x1b8;
			i>>=2;
			j=ed->scroll;
			goto scroll;
		}
		break;
	case WM_VSCROLL:
		ed=(CHOOSEDUNG*)GetWindowLong(win,GWL_USERDATA);
		j=i=ed->scroll;
		switch(wparam&65535) {
		case SB_LINEDOWN:
			i++;
			break;
		case SB_LINEUP:
			i--;
			break;
		case SB_PAGEDOWN:
			i+=4;
			break;
		case SB_PAGEUP:
			i-=4;
			break;
		case SB_TOP:
			i=0;
			break;
		case SB_BOTTOM:
			i=0x6d;
			break;
		case SB_THUMBTRACK:
		case SB_THUMBPOSITION:
			i=wparam>>16;
		}
scroll:
		if(ed->ed->selchk&1) {
			if(i<0x6e) i=0x6e;
			if(i>0x94) i=0x94;
		} else {
			if(i<0) i=0;
			if(i>0x6a) i=0x6a;
		}
		if(i==j) break;
		ed->scroll=i;
		if(j<i-4) j=i-4;
		if(j>i+4) j=i+4;
		si.cbSize=sizeof(si);
		si.fMask=SIF_POS;
		si.nPos=i;
		SetScrollInfo(win,SB_VERT,&si,1);
		ScrollWindowEx(win,0,(j-i)*ed->h>>2,0,0,0,0,SW_INVALIDATE);
		i<<=2;
		j<<=2;
		if(j<i) j+=16,k=i+16; else k=j,j=i;
		oldpal=SelectPalette(objdc,ed->ed->hpal,0);
		for(;j<k;j++) {
			n=j-i;
			m=(j&3)*ed->w>>2;
			l=((j>>2)&3)*ed->h>>2;
			Rectangle(ed->bufdc,m,l,m+(ed->w>>2),l+(ed->h>>2));
			Updateobjdisplay(ed,n);
		}
		SelectPalette(objdc,oldpal,1);
		break;
	case WM_PAINT:
		ed=(CHOOSEDUNG*)GetWindowLong(win,GWL_USERDATA);
		if(!ed) break;
		hdc=BeginPaint(win,&ps);
		oldpal=SelectPalette(hdc,ed->ed->hpal,1);
		RealizePalette(hdc);
		k=(ed->scroll&3)*ed->h>>2;
		i=ed->h-k;
		if(i>ps.rcPaint.bottom) j=ps.rcPaint.bottom; else j=i;
		if(j>ps.rcPaint.top)
			BitBlt(hdc,ps.rcPaint.left,ps.rcPaint.top,ps.rcPaint.right-ps.rcPaint.left,j-ps.rcPaint.top,ed->bufdc,ps.rcPaint.left,ps.rcPaint.top+k,SRCCOPY);
		if(i<ps.rcPaint.top) j=ps.rcPaint.top; else j=i;
		if(j<ps.rcPaint.bottom)
			BitBlt(hdc,ps.rcPaint.left,j,ps.rcPaint.right-ps.rcPaint.left,ps.rcPaint.bottom-j,ed->bufdc,ps.rcPaint.left,j+k-ed->h,SRCCOPY);
		i=ed->sel-(ed->scroll<<2);
		if(i>=0 && i<16) {
			Getdungselrect(i,&rc,ed);
			FrameRect(hdc,&rc,green_brush);
		}
		SelectPalette(hdc,oldpal,0);
		EndPaint(win,&ps);
		break;
	case WM_LBUTTONDOWN:
		ed=(CHOOSEDUNG*)GetWindowLong(win,GWL_USERDATA);
		SetFocus(win);
		i=ed->sel-(ed->scroll<<2);
		if(i>=0 && i<16) {
			Getdungselrect(i,&rc,ed);
			InvalidateRect(win,&rc,0);
		}
		i=((lparam&65535)<<2)/ed->w;
		i|=(((lparam>>16)<<2)/ed->h)<<2;
		ed->sel=i+(ed->scroll<<2);
		if(i>=0 && i<16) {
			Getdungselrect(i,&rc,ed);
			InvalidateRect(win,&rc,0);
		}
		EnableWindow(GetDlgItem(ed->dlg,IDC_BUTTON1),dp_tab[ed->sel]);
		break;
	case WM_LBUTTONDBLCLK:
		SendMessage(GetParent(win),WM_COMMAND,IDOK,0);
		break;
	default:
		return DefWindowProc(win,msg,wparam,lparam);
	}
}
BOOL CALLBACK choosedung(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	HWND hc;
	HDC hdc;
	CHOOSEDUNG *ed;
	DUNGEDIT *de;
	RECT rc;
	SCROLLINFO si;
	HPALETTE oldpal;
	HWND *p;
	
	int i;
	
	switch(msg) 
	{
	case WM_QUERYNEWPALETTE:
		ed=(CHOOSEDUNG*)GetWindowLong(win,DWL_USER);
		Setpalette(win,ed->ed->hpal);
	
		return 1;
	
	case WM_PALETTECHANGED:
		InvalidateRect(GetDlgItem(win,IDC_CUSTOM1),0,0);
	
		break;
	
	case WM_INITDIALOG:
		ed=malloc(sizeof(CHOOSEDUNG));
		ed->dlg=win;
		de=ed->ed=(DUNGEDIT*)lparam;
	
		if(de->selchk&1) 
		{
			getdoor(de->buf+de->selobj,de->ew.doc->rom);
			i=dm_k*42+0x1b8+dm_l;
		} 
		else 
		{
			getobj(de->buf+de->selobj);
		
			if(dm_k<0xf8) 
				i = dm_k+0x40;
			else if(dm_k<0x100) 
				i = (dm_k-0xf8<<4)+dm_l+0x138;
			else 
				i = dm_k-0x100;
		}
	
		ed->sel=i;
		ed->typednum=0;
		
		i >>= 2;
		if(de->selchk&1) 
		{ 
			if(i>0x94) 
				i=0x94; 
		} 
		else if(i>0x6a) 
			i=0x6a;
		
		ed->scroll=i;
		hdc=GetDC(win);
		hc=GetDlgItem(win,IDC_CUSTOM1);
		GetClientRect(hc,&rc);
		ed->w=rc.right;
		ed->h=rc.bottom;
		ed->bufdc=CreateCompatibleDC(hdc);
		ed->bufbmp=CreateCompatibleBitmap(hdc,rc.right,rc.bottom);
		ReleaseDC(win,hdc);
		SetBkMode(ed->bufdc,TRANSPARENT);
		SelectObject(ed->bufdc,ed->bufbmp);
		SetWindowLong(win,DWL_USER,(long)ed);
		SetWindowLong(hc,GWL_USERDATA,(long)ed);
		si.cbSize=sizeof(si);
		si.fMask=SIF_PAGE|SIF_RANGE|SIF_POS;
		si.nPos=ed->scroll;
		
		if(de->selchk&1) 
		{
			si.nMin=0x6e;
			si.nMax=0x97;
		} 
		else 
		{
			si.nMin=0;
			si.nMax=0x6d;
		}
		
		si.nPage=4;
		SetScrollInfo(hc,SB_VERT,&si,0);
		SelectObject(ed->bufdc,black_brush);
		Rectangle(ed->bufdc,0,0,ed->w,ed->h);
		oldpal=SelectPalette(objdc,de->hpal,0);
		SelectPalette(ed->bufdc,de->hpal,0);
		SelectPalette(objdc,oldpal,1);
		SelectPalette(ed->bufdc,oldpal,1);
		EnableWindow(GetDlgItem(win,IDC_BUTTON1),dp_tab[ed->sel]);
upddisp:
		
		for(i=0;i<16;i++) 
			Updateobjdisplay(ed,i);
		
		InvalidateRect(hc,0,0);
		
		break;
	
	case WM_DESTROY:
		ed = (CHOOSEDUNG*) GetWindowLong(win,DWL_USER);
		DeleteDC(ed->bufdc);
		DeleteObject(ed->bufbmp);
		free(ed);
	
		break;
	
	case WM_COMMAND: // handle dialogue commands
		switch(wparam) 
		{
		case IDOK:
			ed = (CHOOSEDUNG*) GetWindowLong(win,DWL_USER);
			EndDialog(win, ed->sel);
		
			break;
		
		case IDCANCEL:
			EndDialog(win,-1);

			break;
		
		case IDC_BUTTON1:
			ed = (CHOOSEDUNG*) GetWindowLong(win,DWL_USER);
			dunged = ed->ed;
			
			if(DialogBoxParam(hinstance,(LPSTR)IDD_DIALOG22,win,dpcdlgproc,ed->sel)) 
			{
				p = dunged->ew.doc->ents;
				
				for(i=0;i<168;i++,p++) 
				{	if(*p) 
					{
						de = (DUNGEDIT*)GetWindowLong(*p,GWL_USERDATA);
						Updatemap(de);
						InvalidateRect(GetDlgItem(de->dlg,3011),0,0);
					}
				}
				
				hc=GetDlgItem(win,IDC_CUSTOM1);
				
				goto upddisp;
			}

			break;
		}
	}
	return FALSE;
}



char *Getoverstring(OVEREDIT *ed, int t, int n)
{
	int a;
	switch(t) {
	case 3:
		wsprintf(buffer,"%02X",ed->ew.doc->rom[0xdbb73+n]);
		break;
	case 5:
		a=ed->ebuf[ed->sprset][n+2];
		wsprintf(buffer,"%02X-%s",a,sprname[a]);
		break;
	case 7:
		wsprintf(buffer,"%04X",((unsigned short*)(ed->ew.doc->rom+0x15d8a))[n]);
		break;
	case 8:
		wsprintf(buffer,"%02X",ed->ew.doc->rom[0xdb84c+n]);
		break;
	case 9:
		if(n>8) wsprintf(buffer,"Whirl-%02X",((unsigned short*)(ed->ew.doc->rom+0x16cf8))[n-9]);
			else wsprintf(buffer,"Fly-%d",n+1);
		break;
	case 10:
		wsprintf(buffer,"%02X-",ed->sbuf[n+2]);
		strcpy(buffer+3,Getsecretstring(ed->ew.doc->rom,ed->sbuf[n+2]));
		break;
	default:
		wsprintf(buffer,"Invalid object!");
	}
	return buffer;
}

void Overselchg(OVEREDIT *ed, HWND win)
{
	RECT rc;
	if(ed->selobj==-1) return;
	rc.left=ed->objx-(ed->mapscrollh<<5);
	rc.top=ed->objy-(ed->mapscrollv<<5);
	Getstringobjsize(Getoverstring(ed,ed->tool,ed->selobj),&rc);
	InvalidateRect(win,&rc,0);
}
const static short tool_ovt[]={0,0,0,1,0,0,0,2,3,5,4};

void Overtoolchg(OVEREDIT*ed,int i,HWND win)
{
	HWND hc=GetDlgItem(win,3001);
	if(tool_ovt[i]!=tool_ovt[ed->tool]) InvalidateRect(hc,0,0);
	else Overselchg(ed,hc);
	ed->tool=i;
	ed->selobj=-1;
	Overselchg(ed,hc);
}
const char *amb_str[9]={
	"Nothing",
	"Heavy rain",
	"Light rain",
	"Stop",
	"Earthquake",
	"Wind",
	"Flute",
	"Chime 1",
	"Chime 2"
};
BOOL CALLBACK editdungprop(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	int i,j,k,l;
	unsigned char*rom;
	static int radio_ids[3]={IDC_RADIO10,IDC_RADIO12,IDC_RADIO13};
	switch(msg) {
	case WM_INITDIALOG:
		rom=dunged->ew.doc->rom;
		j=dunged->ew.param;
		if(j<0x85) {
			i=rom[0x15510+j];
			if(i==2) CheckDlgButton(win,IDC_RADIO9,BST_CHECKED);
			else if(i) CheckDlgButton(win,IDC_RADIO3,BST_CHECKED);
			else CheckDlgButton(win,IDC_RADIO1,BST_CHECKED);
		} else {
			ShowWindow(GetDlgItem(win,IDC_RADIO9),SW_HIDE);
			ShowWindow(GetDlgItem(win,IDC_RADIO3),SW_HIDE);
			ShowWindow(GetDlgItem(win,IDC_RADIO1),SW_HIDE);
			ShowWindow(GetDlgItem(win,IDC_STATIC2),SW_HIDE);
			ShowWindow(GetDlgItem(win,IDC_STATIC3),SW_SHOW);
			ShowWindow(GetDlgItem(win,IDC_EDIT1),SW_SHOW);
			SetDlgItemInt(win,IDC_EDIT1,((short*)(rom+0x15b36))[j],0);
		}
		i=rom[(j>=0x85?0x15b98:0x15595)+j];
		if(i&15) CheckDlgButton(win,IDC_RADIO5,BST_CHECKED);
		else CheckDlgButton(win,IDC_RADIO4,BST_CHECKED);
		SetDlgItemInt(win,IDC_EDIT2,i>>4,0);
		i=rom[(j>=0x85?0x15b9f:0x1561a)+j];
		if(i&32) CheckDlgButton(win,IDC_CHECK1,BST_CHECKED);
		if(i&2) CheckDlgButton(win,IDC_CHECK4,BST_CHECKED);
		switch(l=rom[(j>=0x85?0x15ba6:0x1569f)+j]) {
		case 0:
			CheckDlgButton(win,IDC_RADIO6,BST_CHECKED);
			break;
		case 2:
			CheckDlgButton(win,IDC_RADIO8,BST_CHECKED);
			break;
		case 16:
			CheckDlgButton(win,IDC_RADIO7,BST_CHECKED);
			break;
		case 18:
			CheckDlgButton(win,IDC_RADIO11,BST_CHECKED);
			break;
		}
		SetDlgItemInt(win,IDC_EDIT3,(char)rom[(j>=0x85?0x15b8a:0x15406)+j],1);
		i=((unsigned short*)(rom+(j>=0x85?0x15b28:0x15724)))[j];
		if(i && i!=65535) {
			k=(i>>15)+1;
			SetDlgItemInt(win,IDC_EDIT13,(i&0x7e)>>1,0);
			SetDlgItemInt(win,IDC_EDIT14,(i&0x3f80)>>7,0);
		} else {
			k=0;
			EnableWindow(GetDlgItem(win,IDC_EDIT13),0);
			EnableWindow(GetDlgItem(win,IDC_EDIT14),0);
		}
		CheckDlgButton(win,radio_ids[k],BST_CHECKED);
		j<<=3;
		if(j>=0x428) j+=0xe37;
		SetDlgItemInt(win,IDC_EDIT4,rom[0x1491d+j],0);
		SetDlgItemInt(win,IDC_EDIT5,rom[0x1491e+j],0);
		SetDlgItemInt(win,IDC_EDIT22,rom[0x1491f+j],0);
		SetDlgItemInt(win,IDC_EDIT23,rom[0x14920+j],0);
		SetDlgItemInt(win,IDC_EDIT24,rom[0x14921+j],0);
		SetDlgItemInt(win,IDC_EDIT25,rom[0x14922+j],0);
		SetDlgItemInt(win,IDC_EDIT26,rom[0x14923+j],0);
		SetDlgItemInt(win,IDC_EDIT27,rom[0x14924+j],0);
		break;
	case WM_COMMAND:
		switch(wparam) {
		case IDC_RADIO10:
			EnableWindow(GetDlgItem(win,IDC_EDIT13),0);
			EnableWindow(GetDlgItem(win,IDC_EDIT14),0);
			break;
		case IDC_RADIO12:
		case IDC_RADIO13:
			EnableWindow(GetDlgItem(win,IDC_EDIT13),1);
			EnableWindow(GetDlgItem(win,IDC_EDIT14),1);
			break;
		case IDOK:
			rom=dunged->ew.doc->rom;
			j=dunged->ew.param;
			if(j<0x85) {
				if(IsDlgButtonChecked(win,IDC_RADIO9)) i=2;
				else if(IsDlgButtonChecked(win,IDC_RADIO3)) i=1;
				else i=0;
				rom[0x15510+j]=i;
			} else ((short*)(rom+0x15b36))[j]=GetDlgItemInt(win,IDC_EDIT1,0,0);
			i=IsDlgButtonChecked(win,IDC_RADIO5);
			rom[(j>=0x85?0x15b98:0x15595)+j]=i+(GetDlgItemInt(win,IDC_EDIT2,0,0)<<4);
			i=0;
			if(IsDlgButtonChecked(win,IDC_CHECK4)) i=2;
			if(IsDlgButtonChecked(win,IDC_CHECK1)) i|=32;
			rom[(j>=0x85?0x15b9f:0x1561a)+j]=i;
			if(IsDlgButtonChecked(win,IDC_RADIO6)) i=0;
			else if(IsDlgButtonChecked(win,IDC_RADIO8)) i=2;
			else if(IsDlgButtonChecked(win,IDC_RADIO7)) i=16;
			else i=18;
			rom[(j>=0x85?0x15ba6:0x1569f)+j]=i;
			rom[(j>=0x85?0x15b8a:0x15406)+j]=GetDlgItemInt(win,IDC_EDIT3,0,1);
			if(IsDlgButtonChecked(win,IDC_RADIO10)) i=-1;
			else {
				i=(GetDlgItemInt(win,IDC_EDIT13,0,0)<<1)+(GetDlgItemInt(win,IDC_EDIT14,0,0)<<7);
				if(IsDlgButtonChecked(win,IDC_RADIO13)) i|=0x8000;
			}
			((short*)(rom+(j>=0x85?0x15b28:0x15724)))[j]=i;
			i=((short*)(rom+(j>=0x85?0x15a64:0x14813)))[j];
			k=(i&496)<<5;
			i=(i&15)<<9;
			j<<=3;
			if(j>=0x428) j+=0xe37;
			rom[0x1491d+j]=GetDlgItemInt(win,IDC_EDIT4,0,0);
			rom[0x1491e+j]=GetDlgItemInt(win,IDC_EDIT5,0,0);
			rom[0x1491f+j]=GetDlgItemInt(win,IDC_EDIT22,0,0);
			rom[0x14920+j]=GetDlgItemInt(win,IDC_EDIT23,0,0);
			rom[0x14921+j]=GetDlgItemInt(win,IDC_EDIT24,0,0);
			rom[0x14922+j]=GetDlgItemInt(win,IDC_EDIT25,0,0);
			rom[0x14923+j]=GetDlgItemInt(win,IDC_EDIT26,0,0);
			rom[0x14924+j]=GetDlgItemInt(win,IDC_EDIT27,0,0);
			dunged->ew.doc->modf=1;
		case IDCANCEL:
			EndDialog(win,0);
			break;
		}
	}
	return FALSE;
}
BOOL CALLBACK editroomprop(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	int i,l;
	HWND hc;
	static int hs;
	unsigned char*rom;
	const static char*ef_str[8]={
		"Nothing","01","Moving floor","Moving water","04","Red flashes","Light torch to see floor","Ganon room"
	};
	const static char*tag_str[64]={
		"Nothing","NW Kill enemy to open","NE Kill enemy to open","SW Kill enemy to open","SE Kill enemy to open","W Kill enemy to open","E Kill enemy to open","N Kill enemy to open","S Kill enemy to open","Clear quadrant to open","Clear room to open",
		"NW Move block to open","NE Move block to open","SW Move block to open","SE Move block to open","W Move block to open","E Move block to open","N Move block to open",
		"S Move block to open","Move block to open","Pull lever to open","Clear level to open door","Switch opens door(Hold)","Switch opens door(Toggle)",
		"Turn off water","Turn on water","Water gate","Water twin","Secret wall (Right)","Secret wall (Left)","Crash","Crash",
		"Use switch to bomb wall","Holes(0)","Open chest for holes(0)","Holes(1)","Holes(2)","Kill enemy to clear level","SE Kill enemy to move block","Trigger activated chest",
		"Use lever to bomb wall","NW Kill enemy for chest","NE Kill enemy for chest","SW Kill enemies for chest","SE Kill enemy for chest","W Kill enemy for chest","E Kill enemy for chest","N Kill enemy for chest",
		"S Kill enemy for chest","Clear quadrant for chest","Clear room for chest","Light torches to open","Holes(3)","Holes(4)","Holes(5)","Holes(6)",
		"Agahnim's room","Holes(7)","Holes(8)","Open chest for holes(8)","Move block to get chest","Kill to open Ganon's door","Light torches to get chest","Kill boss again",
	};
	const static int warp_ids[17]={
		IDC_STATIC2,IDC_STATIC3,IDC_STATIC4,IDC_STATIC5,
		IDC_STATIC6,IDC_STATIC7,IDC_STATIC8,IDC_EDIT4,
		IDC_EDIT6,IDC_EDIT7,IDC_EDIT15,IDC_EDIT16,
		IDC_EDIT17,IDC_EDIT18,IDC_EDIT19,IDC_EDIT20,
		IDC_EDIT21
	};
	const static char warp_flag[20]={
		0x07,0x07,0x07,0x0f,0x1f,0x3f,0x7f,0x07,
		0x07,0x0f,0x0f,0x1f,0x1f,0x3f,0x3f,0x7f,
		0x7f,0,0,0
	};
	switch(msg) {
	case WM_INITDIALOG:
		rom=dunged->ew.doc->rom;
		hc=GetDlgItem(win,IDC_COMBO1);
		for(i=0;i<8;i++) SendMessage(hc,CB_ADDSTRING,0,(long)ef_str[i]);
		SendMessage(hc,CB_SETCURSEL,dunged->hbuf[4],0);
		hc=GetDlgItem(win,IDC_COMBO2);
		for(i=0;i<64;i++) SendMessage(hc,CB_ADDSTRING,0,(long)tag_str[i]);
		SendMessage(hc,CB_SETCURSEL,dunged->hbuf[5],0);
		hc=GetDlgItem(win,IDC_COMBO3);
		for(i=0;i<64;i++) SendMessage(hc,CB_ADDSTRING,0,(long)tag_str[i]);
		SendMessage(hc,CB_SETCURSEL,dunged->hbuf[6],0);
		SendDlgItemMessage(win,IDC_BUTTON1,BM_SETIMAGE,IMAGE_BITMAP,(int)arrows_imgs[2]);
		SendDlgItemMessage(win,IDC_BUTTON3,BM_SETIMAGE,IMAGE_BITMAP,(int)arrows_imgs[3]);
		hs=dunged->hsize;
		SetDlgItemInt(win,IDC_EDIT6,dunged->hbuf[7]&3,0);
		SetDlgItemInt(win,IDC_EDIT15,(dunged->hbuf[7]>>2)&3,0);
		SetDlgItemInt(win,IDC_EDIT17,(dunged->hbuf[7]>>4)&3,0);
		SetDlgItemInt(win,IDC_EDIT19,(dunged->hbuf[7]>>6)&3,0);
		SetDlgItemInt(win,IDC_EDIT21,dunged->hbuf[8]&3,0);
		i=dunged->mapnum&0xff00;
		SetDlgItemInt(win,IDC_EDIT4,dunged->hbuf[9]+i,0);
		SetDlgItemInt(win,IDC_EDIT7,dunged->hbuf[10]+i,0);
		SetDlgItemInt(win,IDC_EDIT16,dunged->hbuf[11]+i,0);
		SetDlgItemInt(win,IDC_EDIT18,dunged->hbuf[12]+i,0);
		SetDlgItemInt(win,IDC_EDIT20,dunged->hbuf[13]+i,0);
		SetDlgItemInt(win,IDC_EDIT1,((short*)(rom+0x3f61d))[dunged->mapnum],0);
		for(i=0;i<57;i++) if(((short*)(rom+0x190c))[i]==dunged->mapnum) {
			CheckDlgButton(win,IDC_CHECK5,BST_CHECKED);
			break;
		}
updbtn:
		l=1<<hs-7;
		for(i=0;i<17;i++) ShowWindow(GetDlgItem(win,warp_ids[i]),(warp_flag[i]&l)?SW_HIDE:SW_SHOW);
		EnableWindow(GetDlgItem(win,IDC_BUTTON1),hs>7);
		EnableWindow(GetDlgItem(win,IDC_BUTTON3),hs<14);
		break;
	case WM_COMMAND:
		switch(wparam) {
		case IDC_BUTTON1:
			hs--;
			if(hs==9) hs=7;
			goto updbtn;
		case IDC_BUTTON3:
			hs++;
			if(hs==8) hs=10;
			goto updbtn;
		case IDC_CHECK5:
			rom=dunged->ew.doc->rom;
			if(IsDlgButtonChecked(win,IDC_CHECK5)) {
				for(i=0;i<57;i++) if(((short*)(rom+0x190c))[i]==-1) {
					((short*)(rom+0x190c))[i]=dunged->mapnum;
					break;
				}
				if(i==57) {
					MessageBox(framewnd,"You can't add anymore.","Bad error happened",MB_OK);
					CheckDlgButton(win,IDC_CHECK5,BST_UNCHECKED);
				}
			} else for(i=0;i<57;i++) if(((short*)(rom+0x190c))[i]==dunged->mapnum) {
				((short*)(rom+0x190c))[i]=-1;
				break;
			}
			break;
		case IDOK:
			rom=dunged->ew.doc->rom;
			dunged->hsize=hs;
			dunged->hbuf[4]=SendDlgItemMessage(win,IDC_COMBO1,CB_GETCURSEL,0,0);
			dunged->hbuf[5]=SendDlgItemMessage(win,IDC_COMBO2,CB_GETCURSEL,0,0);
			dunged->hbuf[6]=SendDlgItemMessage(win,IDC_COMBO3,CB_GETCURSEL,0,0);
			dunged->hbuf[7]=(GetDlgItemInt(win,IDC_EDIT6,0,0)&3)|
			((GetDlgItemInt(win,IDC_EDIT15,0,0)&3)<<2)|
			((GetDlgItemInt(win,IDC_EDIT17,0,0)&3)<<4)|
			((GetDlgItemInt(win,IDC_EDIT19,0,0)&3)<<6);
			dunged->hbuf[8]=GetDlgItemInt(win,IDC_EDIT21,0,0)&3;
			dunged->hbuf[9]=GetDlgItemInt(win,IDC_EDIT4,0,0);
			dunged->hbuf[10]=GetDlgItemInt(win,IDC_EDIT7,0,0);
			dunged->hbuf[11]=GetDlgItemInt(win,IDC_EDIT16,0,0);
			dunged->hbuf[12]=GetDlgItemInt(win,IDC_EDIT18,0,0);
			dunged->hbuf[13]=GetDlgItemInt(win,IDC_EDIT20,0,0);
			((short*)(rom+0x3f61d))[dunged->mapnum]=GetDlgItemInt(win,IDC_EDIT1,0,0);
			dunged->modf=dunged->hmodf=1;
			dunged->ew.doc->modf=1;
		case IDCANCEL:
			EndDialog(win,0);
		}
	}
	return FALSE;
}
BOOL CALLBACK editovprop(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	HWND hc;
	int i,j,k;
	unsigned char*rom;
	int cb_ids[4]={IDC_COMBO1,IDC_COMBO2,IDC_COMBO3,IDC_COMBO7};
	int cb2_ids[4]={IDC_COMBO4,IDC_COMBO5,IDC_COMBO6,IDC_COMBO8};
	int text_ids[4]={IDC_STATIC2,IDC_STATIC3,IDC_STATIC4};
	switch(msg) {
	case WM_INITDIALOG:
		i=0;
		if(oved->ew.param>=0x40) {
			for(;i<3;i++) {
				ShowWindow(GetDlgItem(win,cb_ids[i]),SW_HIDE);
				ShowWindow(GetDlgItem(win,cb2_ids[i]),SW_HIDE);
				ShowWindow(GetDlgItem(win,text_ids[i]),SW_HIDE);
			}
			ShowWindow(GetDlgItem(win,IDC_STATIC5),SW_HIDE);
		}
		rom=oved->ew.doc->rom;
		for(;i<4;i++) {
			k=rom[0x14303+oved->ew.param+(i<<6)];
			if(k>=16) k+=16;
			hc=GetDlgItem(win,cb_ids[i]);
			for(j=0;j<16;j++) SendMessage(hc,CB_ADDSTRING,0,(long)mus_str[j+1]);
			SendMessage(hc,CB_SETCURSEL,k&15,0);
			hc=GetDlgItem(win,cb2_ids[i]);
			for(j=0;j<9;j++) SendMessage(hc,CB_ADDSTRING,0,(long)amb_str[j]);
			SendMessage(hc,CB_SETCURSEL,k>>5,0);
		}
		SetDlgItemInt(win,IDC_EDIT1,((short*)(rom+0x3f51d))[oved->ew.param],0);
		break;
	case WM_COMMAND:
		switch(wparam) {
		case IDOK:
			rom=oved->ew.doc->rom;
			if(oved->ew.param>=0x40) i=3; else i=0;
			for(;i<4;i++) {
				hc=GetDlgItem(win,cb_ids[i]);
				k=SendMessage(hc,CB_GETCURSEL,0,0);
				hc=GetDlgItem(win,cb2_ids[i]);
				k|=SendMessage(hc,CB_GETCURSEL,0,0)<<5;
				if(k>=32) k-=16;
				rom[0x14303+oved->ew.param+(i<<6)]=k;
			}
			((short*)(rom+0x3f51d))[oved->ew.param]=GetDlgItemInt(win,IDC_EDIT1,0,0);
			oved->ew.doc->modf;
		case IDCANCEL:
			EndDialog(win,0);
		}
	}
	return 0;
}
BOOL CALLBACK editexit(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	unsigned char*rom;
	int i,j,k,l,m,n,o;
	HWND hc;
	const static int radio_ids[6]={IDC_RADIO2,IDC_RADIO4,IDC_RADIO5,IDC_RADIO6,IDC_RADIO7,IDC_RADIO8};
	const static int door_ofs[2]={0x16367,0x16405};
	const static int edit_ids[6]={IDC_EDIT8,IDC_EDIT9,IDC_EDIT10,IDC_EDIT11};
	const static int hide_ids[20]={IDC_STATIC2,IDC_STATIC3,IDC_STATIC4,IDC_STATIC5,IDC_STATIC6,
		IDC_EDIT15,IDC_EDIT22,IDC_EDIT23,IDC_EDIT24,IDC_EDIT25,
		IDC_STATIC7,IDC_STATIC8,IDC_STATIC9,IDC_STATIC10,IDC_STATIC11,
		IDC_EDIT26,IDC_EDIT27,IDC_EDIT28,IDC_EDIT29,IDC_EDIT30};
	switch(msg) {
	case WM_INITDIALOG:
		rom=oved->ew.doc->rom;
		i=oved->selobj;
		j=(oved->ew.param&7)<<9;
		k=(oved->ew.param&56)<<6;
		SetDlgItemInt(win,IDC_EDIT1,((short*)(rom+0x15d8a))[i],0);
		SetDlgItemInt(win,IDC_EDIT2,((short*)(rom+0x15f15))[i]-k,1);
		SetDlgItemInt(win,IDC_EDIT3,((short*)(rom+0x15fb3))[i]-j,1);
		SetDlgItemInt(win,IDC_EDIT4,((short*)(rom+0x1618d))[i]-k,1);
		SetDlgItemInt(win,IDC_EDIT5,((short*)(rom+0x1622b))[i]-j,1);
		SetDlgItemInt(win,IDC_EDIT6,((short*)(rom+0x16051))[i]-k,1);
		SetDlgItemInt(win,IDC_EDIT7,((short*)(rom+0x160ef))[i]-j,1);
		for(j=0;j<2;j++) {
			l=((unsigned short*)(rom+door_ofs[j]))[i];
			if(l && l!=65535) {
				m=(l>>15)+1;
				SetDlgItemInt(win,edit_ids[j*2],(l&0x7e)>>1,0);
				SetDlgItemInt(win,edit_ids[j*2+1],(l&0x3f80)>>7,0);
			} else {
				m=0;
				EnableWindow(GetDlgItem(win,edit_ids[j*2]),0);
				EnableWindow(GetDlgItem(win,edit_ids[j*2+1]),0);
			}
			m+=3*j;
			CheckDlgButton(win,radio_ids[m],BST_CHECKED);
		}
		SetDlgItemInt(win,IDC_EDIT12,rom[0x15e28+i],0);
		SetDlgItemInt(win,IDC_EDIT13,(char)rom[0x162c9+i],1);
		SetDlgItemInt(win,IDC_EDIT14,(char)rom[0x16318+i],1);
		break;
	case WM_COMMAND:
		switch(wparam) {
		case IDC_EDIT1|(EN_CHANGE<<16):
			i=GetDlgItemInt(win,IDC_EDIT1,0,0);
			rom=oved->ew.doc->rom;
			if(i>=0x180 && i<0x190) {
				j=SW_SHOW;
				SetDlgItemInt(win,IDC_EDIT15,rom[0x16681+i]>>1,0);
				SetDlgItemInt(win,IDC_EDIT22,rom[0x16691+i],0);
				SetDlgItemInt(win,IDC_EDIT23,rom[0x166a1+i],0);
				SetDlgItemInt(win,IDC_EDIT24,rom[0x166b1+i],0);
				SetDlgItemInt(win,IDC_EDIT25,rom[0x166c1+i],0);
				SetDlgItemInt(win,IDC_EDIT26,((short*)(rom+0x163e1))[i],0);
				SetDlgItemInt(win,IDC_EDIT27,((short*)(rom+0x16401))[i],0);
				SetDlgItemInt(win,IDC_EDIT28,((short*)(rom+0x16421))[i],0);
				SetDlgItemInt(win,IDC_EDIT29,((short*)(rom+0x16441))[i],0);
				SetDlgItemInt(win,IDC_EDIT30,((short*)(rom+0x164e1))[i],0);
			} else j=SW_HIDE;
			for(k=0;k<20;k++)
				ShowWindow(GetDlgItem(win,hide_ids[k]),j);
			break;
		case IDC_RADIO2:
			EnableWindow(GetDlgItem(win,IDC_EDIT8),0);
			EnableWindow(GetDlgItem(win,IDC_EDIT9),0);
			break;
		case IDC_RADIO4: case IDC_RADIO5:
			EnableWindow(GetDlgItem(win,IDC_EDIT8),1);
			EnableWindow(GetDlgItem(win,IDC_EDIT9),1);
			break;
		case IDC_RADIO6:
			EnableWindow(GetDlgItem(win,IDC_EDIT10),0);
			EnableWindow(GetDlgItem(win,IDC_EDIT11),0);
			break;
		case IDC_RADIO7: case IDC_RADIO8:
			EnableWindow(GetDlgItem(win,IDC_EDIT10),1);
			EnableWindow(GetDlgItem(win,IDC_EDIT11),1);
			break;
		case IDOK:
			hc=GetDlgItem(oved->dlg,3001);
			Overselchg(oved,hc);
			rom=oved->ew.doc->rom;
			i=oved->selobj;
			l=rom[0x15e28+i]=GetDlgItemInt(win,IDC_EDIT12,0,0);
			if(l!=oved->ew.param) oved->selobj=-1;
			n=oved->mapsize?1023:511;
			j=(l&7)<<9;
			k=(l&56)<<6;
			((short*)(rom+0x15d8a))[i]=o=GetDlgItemInt(win,IDC_EDIT1,0,0);
			((short*)(rom+0x15f15))[i]=(l=(GetDlgItemInt(win,IDC_EDIT2,0,0)&n))+k;
			((short*)(rom+0x15fb3))[i]=(m=(GetDlgItemInt(win,IDC_EDIT3,0,0)&n))+j;
			((short*)(rom+0x15e77))[i]=((l&0xfff0)<<3)|((m&0xfff0)>>3);
			((short*)(rom+0x1618d))[i]=(GetDlgItemInt(win,IDC_EDIT4,0,0)&n)+k;
			((short*)(rom+0x1622b))[i]=(GetDlgItemInt(win,IDC_EDIT5,0,0)&n)+j;
			((short*)(rom+0x16051))[i]=(oved->objy=GetDlgItemInt(win,IDC_EDIT6,0,0)&n)+k;
			((short*)(rom+0x160ef))[i]=(oved->objx=GetDlgItemInt(win,IDC_EDIT7,0,0)&n)+j;
			m=0;
			for(j=0;j<2;j++) {
				if(IsDlgButtonChecked(win,radio_ids[m])) l=0;
				else {
					l=(GetDlgItemInt(win,edit_ids[j*2],0,0)<<1)+(GetDlgItemInt(win,edit_ids[j*2+1],0,0)<<7);
					if(IsDlgButtonChecked(win,radio_ids[m+2])) l|=0x8000;
				}
				((short*)(rom+door_ofs[j]))[i]=l;
				m+=3;
			}
			rom[0x162c9+i]=GetDlgItemInt(win,IDC_EDIT13,0,1);
			rom[0x16318+i]=GetDlgItemInt(win,IDC_EDIT14,0,1);
			if(o>=0x180 && o<0x190) {
				rom[0x16681+o]=GetDlgItemInt(win,IDC_EDIT15,0,0)<<1;
				rom[0x16691+o]=GetDlgItemInt(win,IDC_EDIT22,0,0);
				rom[0x166a1+o]=GetDlgItemInt(win,IDC_EDIT23,0,0);
				rom[0x166b1+o]=GetDlgItemInt(win,IDC_EDIT24,0,0);
				rom[0x166c1+o]=GetDlgItemInt(win,IDC_EDIT25,0,0);
				((short*)(rom+0x163e1))[o]=GetDlgItemInt(win,IDC_EDIT26,0,0);
				((short*)(rom+0x16401))[o]=GetDlgItemInt(win,IDC_EDIT27,0,0);
				((short*)(rom+0x16421))[o]=GetDlgItemInt(win,IDC_EDIT28,0,0);
				((short*)(rom+0x16441))[o]=GetDlgItemInt(win,IDC_EDIT29,0,0);
				((short*)(rom+0x164e1))[o]=GetDlgItemInt(win,IDC_EDIT30,0,0);
			}
			Overselchg(oved,hc);
			oved->ew.doc->modf=1;
		case IDCANCEL:
			EndDialog(win,0);
			break;
		}
	}
	return FALSE;
}
BOOL CALLBACK editwhirl(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	unsigned char*rom;
	int i,j,k,l,m,n;
	HWND hc;
	switch(msg) {
	case WM_INITDIALOG:
		rom=oved->ew.doc->rom;
		i=oved->selobj;
		j=(oved->ew.param&7)<<9;
		k=(oved->ew.param&56)<<6;
		if(i>8) SetDlgItemInt(win,IDC_EDIT1,((short*)(rom+0x16ce6))[i],0);
		else {
			ShowWindow(GetDlgItem(win,IDC_STATIC2),SW_HIDE);
			ShowWindow(GetDlgItem(win,IDC_EDIT1),SW_HIDE);
		}
		SetDlgItemInt(win,IDC_EDIT2,((short*)(rom+0x16b29))[i]-k,1);
		SetDlgItemInt(win,IDC_EDIT3,((short*)(rom+0x16b4b))[i]-j,1);
		SetDlgItemInt(win,IDC_EDIT4,((short*)(rom+0x16bb1))[i]-k,1);
		SetDlgItemInt(win,IDC_EDIT5,((short*)(rom+0x16bd3))[i]-j,1);
		SetDlgItemInt(win,IDC_EDIT6,((short*)(rom+0x16b6d))[i]-k,1);
		SetDlgItemInt(win,IDC_EDIT7,((short*)(rom+0x16b8f))[i]-j,1);
		SetDlgItemInt(win,IDC_EDIT12,((short*)(rom+0x16ae5))[i],0);
		SetDlgItemInt(win,IDC_EDIT13,(char)rom[0x16bf5+i],1);
		SetDlgItemInt(win,IDC_EDIT14,(char)rom[0x16c17+i],1);
		break;
	case WM_COMMAND:
		switch(wparam) {
		case IDC_RADIO2:
			EnableWindow(GetDlgItem(win,IDC_EDIT8),0);
			EnableWindow(GetDlgItem(win,IDC_EDIT9),0);
			break;
		case IDC_RADIO4: case IDC_RADIO5:
			EnableWindow(GetDlgItem(win,IDC_EDIT8),1);
			EnableWindow(GetDlgItem(win,IDC_EDIT9),1);
			break;
		case IDC_RADIO6:
			EnableWindow(GetDlgItem(win,IDC_EDIT10),0);
			EnableWindow(GetDlgItem(win,IDC_EDIT11),0);
			break;
		case IDC_RADIO7: case IDC_RADIO8:
			EnableWindow(GetDlgItem(win,IDC_EDIT10),1);
			EnableWindow(GetDlgItem(win,IDC_EDIT11),1);
			break;
		case IDOK:
			hc=GetDlgItem(oved->dlg,3001);
			Overselchg(oved,hc);
			rom=oved->ew.doc->rom;
			i=oved->selobj;
			l=((short*)(rom+0x16ae5))[i]=GetDlgItemInt(win,IDC_EDIT12,0,0);
			n=(rom[0x12884+l]<<8)|0xff;
			j=(l&7)<<9;
			k=(l&56)<<6;
			if(l!=oved->ew.param) oved->selobj=-1;
			if(i>8) ((short*)(rom+0x16ce6))[i]=GetDlgItemInt(win,IDC_EDIT1,0,0);
			((short*)(rom+0x16b29))[i]=(l=GetDlgItemInt(win,IDC_EDIT2,0,0)&n)+k;
			((short*)(rom+0x16b4b))[i]=(m=GetDlgItemInt(win,IDC_EDIT3,0,0)&n)+j;
			((short*)(rom+0x16b07))[i]=((l&0xfff0)<<3)|((m&0xfff0)>>3);
			((short*)(rom+0x16bb1))[i]=(GetDlgItemInt(win,IDC_EDIT4,0,0)&n)+k;
			((short*)(rom+0x16bd3))[i]=(GetDlgItemInt(win,IDC_EDIT5,0,0)&n)+j;
			((short*)(rom+0x16b6d))[i]=(oved->objy=GetDlgItemInt(win,IDC_EDIT6,0,0)&n)+k;
			((short*)(rom+0x16b8f))[i]=(oved->objx=GetDlgItemInt(win,IDC_EDIT7,0,0)&n)+j;
			rom[0x16bf5+i]=GetDlgItemInt(win,IDC_EDIT13,0,1);
			rom[0x16c17+i]=GetDlgItemInt(win,IDC_EDIT14,0,1);
			Overselchg(oved,hc);
			oved->ew.doc->modf=1;
		case IDCANCEL:
			EndDialog(win,0);
			break;
		}
	}
	return FALSE;
}
BOOL CALLBACK choosesprite(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	HWND hc;
	int i,j,k;
	switch(msg) {
	case WM_INITDIALOG:
		hc=GetDlgItem(win,IDC_LIST1);
		j=(lparam&512)?0x11c:256;
		for(i=0;i<j;i++) {
			SendMessage(hc,LB_SETITEMDATA,k=SendMessage(hc,LB_ADDSTRING,0,(long)sprname[i]),i);
			if(i==(lparam&511)) SendMessage(hc,LB_SETCURSEL,k,0);
		}
		SendMessage(hc,LB_SETTOPINDEX,SendMessage(hc,LB_GETCURSEL,0,0),0);
		break;
	case WM_COMMAND:
		switch(wparam) {
		case IDOK:
			hc=GetDlgItem(win,IDC_LIST1);
			EndDialog(win,SendMessage(hc,LB_GETITEMDATA,SendMessage(hc,LB_GETCURSEL,0,0),0));
			break;
		case IDCANCEL:
			EndDialog(win,-1);
			break;
		}
	}
	return FALSE;
}
BOOL CALLBACK getnumber(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	int i,j;
	if(msg==WM_INITDIALOG) {
		SetWindowLong(win,DWL_USER,lparam);
		SetWindowText(win,((LPSTR*)lparam)[1]);
		SetDlgItemText(win,IDC_STATIC2,((LPSTR*)lparam)[2]);
		SetFocus(GetDlgItem(win,IDC_EDIT1));
	} else if(msg==WM_COMMAND) if(wparam==IDOK) {
		i=GetDlgItemInt(win,IDC_EDIT1,0,0);
		j=*(int*)GetWindowLong(win,DWL_USER);
		if(i<0 || i>j) {
			wsprintf(buffer,"Please enter a number between 0 and %d",j);
			MessageBox(framewnd,buffer,"Bad error happened",MB_OK);
			return FALSE;
		}
		EndDialog(win,i);
	} else if(wparam==IDCANCEL) EndDialog(win,-1);
	return FALSE;
}
int __stdcall askinteger(int max,char*caption,char*text)
{
	return ShowDialog(hinstance,(LPSTR)IDD_DIALOG4,framewnd,getnumber,(int)&max);
}
static char*screen_text[]={
	"Title screen",
	"Naming screen",
	"Player select",
	"Copy screen",
	"Erase screen",
	"Map screen",
	"Load list",
	"Load screen",
	"Erase list",
	"Copy list",
	"Destination list"
};

static char*level_str[]={
	"None",
	"Church",
	"Castle",
	"East",
	"Desert",
	"Agahnim",
	"Water",
	"Dark",
	"Mud",
	"Wood",
	"Ice",
	"Tower",
	"Town",
	"Mountain",
	"Agahnim 2"
};

//Setpalmode#*******************************

void Setpalmode(DUNGEDIT *ed)
{
	int i,
		j,
		k,
		t = ed->paltype;

	struct
	{
		short blah1, blah2;
		int pal[256];
	} palstr;

	palstr.blah1 = 0x300;
	palstr.blah2 = palt_sz[t];

	k = palt_st[t];

	for(i = 0; i < palstr.blah2; i++)
	{
		j = *(int*)(ed->pal + i + k);
		palstr.pal[i] = ((j & 0xFF) << 16) | ((j & 0xFF0000) >> 16) | (j & 0xFF00);
	}
	
	ed->hpal = CreatePalette((LOGPALETTE*)&palstr);
	
	for(i = 0; i < 256; i++)
		((short*)(&(ed->pal)))[i] = (i - k) & 255;
}

//Setpalmode********************************

//Setfullmode#******************************

void Setfullmode(DUNGEDIT *ed)
{
	int pal[256];

	int i,
		j,
		k = ed->paltype,
		l;
	
	l = i = palt_st[k];
	j = palt_sz[k];
	
	GetPaletteEntries(ed->hpal,i,j,(PALETTEENTRY*) pal);
	
	for( ; i < j; i++) 
	{
		k = pal[i];
		
		*(int*) (ed->pal+i+l) = (k >> 16) | ((k & 0xff) << 16) | (k & 0xff0000);
	}
	
	DeleteObject(ed->hpal);
	ed->hpal = 0;
}

//Setfullmode*******************************

//Addgraphwin#*******************************

void Addgraphwin(DUNGEDIT *ed, int t)
{
	ed->prevgraph = lastgraph;

	if(lastgraph) 
		((DUNGEDIT*) lastgraph)->nextgraph = ed;
	else 
		firstgraph = ed;
	
	lastgraph = ed;
	
	ed->nextgraph = 0;
	ed->paltype = t;
	
	if(palmode) 
		Setpalmode(ed);
}

//Addgraphwin********************************

void Delgraphwin(DUNGEDIT*ed)
{
	if(ed->hpal) 
		DeleteObject(ed->hpal);
	
	if(dispwnd == ed)
		dispwnd = ed->prevgraph;
	
	if(ed->prevgraph) 
		((DUNGEDIT*)ed->prevgraph)->nextgraph = ed->nextgraph;
	else 
		firstgraph = ed->nextgraph;
	
	if(ed->nextgraph) 
		((DUNGEDIT*)ed->nextgraph)->prevgraph = ed->prevgraph;
	else 
		lastgraph = ed->prevgraph;
	
	if(!dispwnd) 
		dispwnd = lastgraph;
}

void Updpal(void*ed)
{
	if(ed == dispwnd && ((DUNGEDIT*)ed)->hpal) 
		Setpalette(framewnd,((DUNGEDIT*)ed)->hpal);
	else 
		SendMessage(((DUNGEDIT*)ed)->dlg, 4002, 0, 0);
}

void Setdispwin(DUNGEDIT*ed)
{
	if(ed != dispwnd)
	{
		dispwnd = ed;
		Setpalette( framewnd, ed->hpal);
	}
}


void FixEntScroll(FDOC*doc,int j)
{
	unsigned char*rom=doc->rom;
	HWND win;
	int i,k,l,n,o,p;
	const static unsigned char hfl[4]={0xd0,0xb0};
	const static unsigned char vfl[4]={0x8a,0x86};
	k=((short*)(rom+(j>=0x85?0x15a64:0x14813)))[j];
	if(win=doc->dungs[k]) n=GetDlgItemInt(win,3029,0,0);
	else n=rom[romaddr(*(int*)(rom+0xf8000+k*3))+1]>>2;
	o=rom[(j>=0x85?0x15ac7:0x14f5a)+(j<<1)]&1;
	p=rom[(j>=0x85?0x15ad5:0x15064)+(j<<1)]&1;
	i=(((vfl[o]>>n)<<1)&2)+(((hfl[p]>>n)<<5)&32);
	rom[(j>=0x85?0x15b9f:0x1561a)+j]=i;
	rom[(j>=0x85?0x15ba6:0x1569f)+j]=(o<<1)+(p<<4);
	i=(k>>4)<<1;
	l=(k&15)<<1;
	j<<=3;
	if(j>=0x428) j+=0xe37;
	rom[0x1491d+j]=i+o;
	rom[0x1491e+j]=i;
	rom[0x1491f+j]=i+o;
	rom[0x14920+j]=i+1;
	rom[0x14921+j]=l+p;
	rom[0x14922+j]=l;
	rom[0x14923+j]=l+p;
	rom[0x14924+j]=l+1;
}


//dungdlgproc*********************************

// goal 1
BOOL CALLBACK dungdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	int j,k,i,l,m,n;

	DUNGEDIT *ed; // get us a dungedit pointer.
	HWND hc; // and a window for it.

	char *buf2; // a char buffer.

	unsigned char *rom; // a particular romfile to work with.

//	const static int inf_ofs[5]={0x14813,0x14f59,0x15063,0x14e4f,0x14d45};
//	const static int inf_ofs2[5]={0x156be,0x15bb4,0x15bc2,0x15bfa,0x15bec};
	
	const static unsigned char mus_ofs[] =
	{
		255,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,240,241,242,243
	}; // music offsets?

	const static int ovlhide[] =
	{
		0,1,2,3,4,5,6,7,8,9,10,12,14,15,16,17,20,21,22,23,24,25,26,27,28,29,34,35,36,37,38,39,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60
	};// size = 49
	
	const static char *bg2_str[] = 
	{
		"Off",
		"Parallaxing",
		"Dark",
		"On top",
		"Translucent",
		"Parallaxing2",
		"Normal",
		"Addition",
		"Dark room"
	};

	const static char *coll_str[] = 
	{
		"One",
		"Both",
		"Both w/scroll",
		"Moving floor",
		"Moving water"
	};

	// message handling for the window.
	
	switch(msg) 
	{
	
	case WM_INITDIALOG: // if we need to initialize the dialog...
		
		SetWindowLong(win,DWL_USER,lparam);
		
		ed = (DUNGEDIT*) lparam; // typecast lparam as a pointer to a dungedit.
		
		j = ed->ew.param; // get the 'room' we're looking at.
		
		rom = ed->ew.doc->rom; // get the romfile from ed.
		
		ed->init = 1; // the dungedit has been initialized.
		ed->dlg = win; // pass this win to ed's dialogue.
		ed->hpal = 0; // void the HPALETTE in ed.
		
		if(j >= 0x8c) // if we're dealing with an overlay.
		{
			for(i=0; i < 49; i++) 
				ShowWindow( GetDlgItem(win, ovlhide[i] + 3000),SW_HIDE );
			
			if(j < 0x9f) 
				buf2 = rom + romaddr( *(int*) (rom + 0x26b1c + j*3) );
			else if(j < 0xa7)
				buf2 = rom + 
						romaddr( *(int*) (rom + romaddr ( *(int*) (rom + 0x882d) ) + (j - 0x9f)*3 ) );
			else 
				buf2 = rom + (rom[0x9c25] << 15) - 0x8000 + *(unsigned short*) (rom + 0x9c2a);
			
			for(i=0; ;i += 3) 
				if(*(short*) (buf2 + i) == -1) 
					break;
			
			ed->chkofs[0] = 2;
			ed->len = ed->chkofs[2] = ed->chkofs[1] = i+4;
			ed->selobj = 2;
			ed->buf = malloc(i+4);
			
			*(short*)(ed->buf)=15;
			
			memcpy(ed->buf+2,buf2,i+2);
			ed->hbuf[0] = 0;
			ed->hbuf[1] = 1;
			ed->hbuf[2] = 0;
			ed->hbuf[3] = 0;
			ed->hbuf[4] = 0;
			ed->gfxtmp = 0;

			CheckDlgButton(win,3030,BST_CHECKED);
			CheckDlgButton(win,3031,BST_CHECKED);
			
			Initroom(ed,win);
		} 
		else 
		{
			k = ( (short*) ( rom+(j >= 0x85 ? 0x15a64 : 0x14813 ) ) )[j];
			SetDlgItemInt(win,3002,k,0); // read in the room number, put it to screen.
			i = (k & 496) << 5;
			l = (k & 15) << 9;
		
			if(j >= 0x85) // these are the starting location maps
			{	
				SetDlgItemInt(win,3004,( (short*) (rom+0x15ac6) )[j] - i,0);
				SetDlgItemInt(win,3006,( (short*) (rom+0x15ad4) )[j] - l,0);
				SetDlgItemInt(win,3008,( (short*) (rom+0x15ab8) )[j] - i,0);
				SetDlgItemInt(win,3010,( (short*) (rom+0x15aaa) )[j] - l,0);
				SetDlgItemInt(win,3052,( (short*) (rom+0x15af0) )[j],0);
				SetDlgItemInt(win,3054,( (short*) (rom+0x15ae2) )[j],0);
			} 
			else // these are normal rooms 
			{
				SetDlgItemInt(win,3004,((short*)(rom+0x14f59))[j]-i,0);
				SetDlgItemInt(win,3006,((short*)(rom+0x15063))[j]-l,0);
				SetDlgItemInt(win,3008,((short*)(rom+0x14e4f))[j]-i,0);
				SetDlgItemInt(win,3010,((short*)(rom+0x14d45))[j]-l,0);
				SetDlgItemInt(win,3052,((short*)(rom+0x15277))[j],0);
				SetDlgItemInt(win,3054,((short*)(rom+0x1516d))[j],0);
			}
			
			CheckDlgButton(win,3030,BST_CHECKED);// the show BG1, BG2, Sprite, check boxes.
			CheckDlgButton(win,3031,BST_CHECKED);// make them initially checked.
			CheckDlgButton(win,3048,BST_CHECKED);
	
			for(i = 0; i < 4; i++)
				SendDlgItemMessage(win,3014+i,BM_SETIMAGE,IMAGE_BITMAP,(int)arrows_imgs[i]);
				// map in the arrow bitmaps for those buttons at the top l.eft

			for(i = 0; i < 40; i++)
				SendDlgItemMessage(win,3025,CB_ADDSTRING,0,(int)mus_str[i]);
				// map in the names of the music tracks
				
			l = rom[ (j >= 0x85 ? 0x15bc9 : 0x1582e) + j ];
			// if j is a starting location, use 0x15bc9, otherwise 0x1582e as an offset.
		
			for(i = 0; i < 40; i++)// go through all 
			{	if(mus_ofs[i] == l)
				{
					SendDlgItemMessage(win,3025,CB_SETCURSEL,i,0);
			
					break;
				}
			}
		

			for(i = 0; i < 9; i++)
				SendDlgItemMessage(win,3039,CB_ADDSTRING,0,(int)bg2_str[i]);
		
			for(i = 0; i < 5; i++)
				SendDlgItemMessage(win,3045,CB_ADDSTRING,0,(int)coll_str[i]);
		
			for(i = 0; i < 15; i++)
				SendDlgItemMessage(win,3027,CB_ADDSTRING,0,(int)level_str[i]);
		
			SendDlgItemMessage(	win,
								3027,
								CB_SETCURSEL,
								( (rom[ (j >= 0x85 ? 0x15b91 : 0x1548b) + j] + 2) >> 1) & 0x7f,
								0);
		
			ed->gfxtmp = rom[(j >= 0x85 ? 0x15b83 : 0x15381) + j];
		
			SetDlgItemInt(win, 3023, ed->gfxtmp, 0);
			Openroom(ed,k);
		}
		
		ed->pal[0] = blackcolor;
		
		for(i = 16;i < 256;i += 16)
			ed->pal[i] = ed->pal[0];
		
		ed->mapscrollh = 0;
		ed->mapscrollv = 0;
		ed->bmih = zbmih;
		ed->selchk = 0;
		ed->withfocus = 0;
		ed->disp = 7;
		ed->anim = 0;
		ed->init = 0;
		
		Getblocks(ed->ew.doc, 0x79);
		Getblocks(ed->ew.doc, 0x7a);
		
		Addgraphwin(ed,1);
		Updatemap(ed);
		
		goto updpal3;
	
	case 4002: // used as a code to update the palette? so it seems

		InvalidateRect(GetDlgItem(win,3011),0,0);
		break;
	
	case WM_COMMAND:
		
		ed = (DUNGEDIT*) GetWindowLong(win,DWL_USER);
		
		if(!ed || ed->init)
			break;
		
		switch(wparam) 
		{
		
		// the arrow buttons
		case 3014:
		case 3015:
		case 3016:
		case 3017:
		
			k = ed->mapnum;
			
			k=( k + nxtmap[wparam-3014]&0xff)+(k&0x100);
			
			if(ed->ew.doc->dungs[k]) 
			{
				MessageBox(framewnd,"The room is already open in another editor","Bad error happened",MB_OK);
				break;
			}
newroom:

			if(Closeroom(ed)) 
				break;
			
			ed->init = 1;
			
			Openroom(ed, k);
			
			ed->init = 0;
			Updatemap(ed);
			hc = GetDlgItem(win, 3011);
			Dungselectchg(ed, hc, 1);
			InvalidateRect(hc,0,0);
			
			goto updpal;
		
		case 3047:
			k = askinteger(295,"Jump to room","Room #");
			
			if(k == -1)
				break;
			goto newroom;
		
		case 3030:
			ed->disp &= -2;
			
			if(IsDlgButtonChecked(win,3030) == BST_CHECKED) ed->disp |= 1;
			
			goto updscrn;
		
		case 3031:
			ed->disp &= -3;
			
			if(IsDlgButtonChecked(win,3031) == BST_CHECKED) ed->disp |= 2;
			
			goto updscrn;
		
		case 3048:
			ed->disp &= -5;
			
			if(IsDlgButtonChecked(win,3048) == BST_CHECKED) ed->disp |= 4;
			
			goto updscrn;
		
		case 3033:
			ed->anim++;
			
			if(ed->anim == 3)
				ed->anim = 0;
			
			wsprintf(buffer, "Frm%d", ed->anim + 1);
	
			SetWindowText((HWND)lparam,buffer);
			
			goto updscrn;
		
		case 3034:
			i = 0;
updchk:
			hc = GetDlgItem(win, 3011);
			
			Dungselectchg(ed, hc, 0);

			if(ed->chkofs[i + 1] == ed->chkofs[i] + 2)
			{
				i++;
				
				if(ed->chkofs[i + 1] <= ed->chkofs[i]+2)
				{
					i--,
					ed->selobj = 0;
					goto no_obj;
				}
			}
			
			ed->selobj = ed->chkofs[i];
no_obj:
			ed->selchk = i;
			
			Dungselectchg(ed,hc,1);
			
			break;
		
		case 3035:
			i = 2;
			goto updchk;
		case 3036:
			i = 4;
			goto updchk;
		case 3046:
			hc = GetDlgItem(win,3011);
			
			Dungselectchg(ed,hc,0);
			
			ed->selchk = 6;
			ed->selobj = ed->esize > 2;
			
			Dungselectchg(ed,hc,1);
			
			break;
		case 3002 | (EN_CHANGE << 16):
		case 3004 | (EN_CHANGE << 16):
		case 3006 | (EN_CHANGE << 16):
		case 3008 | (EN_CHANGE << 16):
		case 3010 | (EN_CHANGE << 16):
		case 3019 | (EN_CHANGE << 16):
		case 3021 | (EN_CHANGE << 16):
		case 3023 | (EN_CHANGE << 16):
		case 3029 | (EN_CHANGE << 16):
		case 3041 | (EN_CHANGE << 16):
		case 3043 | (EN_CHANGE << 16):
		case 3050 | (EN_CHANGE << 16):
		case 3052 | (EN_CHANGE << 16):
		case 3054 | (EN_CHANGE << 16):
			
			wparam &= 65535;
			
			j = GetDlgItemInt(win,wparam,0,0);
			
			rom = ed->ew.doc->rom;
			
			if(wparam > 3010)
				switch(wparam)
				{
				case 3019:
		
					if(j>15)
					{
						SetDlgItemInt(win,3019,15,0);
						break;
					}
					else if(j < 0)
					{
						SetDlgItemInt(win,3019,0,0);
						break;
					}
				
					ed->buf[0] &= 0xf0;
					ed->buf[0] |= j;
				
					if(ed->ew.param < 0x8c) 
						ed->modf = 1;
updmap:
				
					Updatemap(ed);
updscrn:
	
					hc = GetDlgItem(win,3011);
				
					InvalidateRect(hc,0,0);
				
					break;
			
				case 3021:
				
					if(j > 15)
					{
						SetDlgItemInt(win, 3021, 15, 0);
						break;
					}
					else if(j < 0)
					{
						SetDlgItemInt(win, 3021, 0, 0);
						break;
					}
				
					ed->buf[0]&=0xf;
					ed->buf[0]|=j<<4;
					ed->modf=1;
				
					goto updmap;
			
				case 3029:
				if(j > 7)
				{
					SetDlgItemInt(win, 3029, 7, 0);
					break;
				}
				else if(j < 0) 
				{
					SetDlgItemInt(win, 3029, 0, 0);
					break;
				}

				ed->buf[1] = j << 2;
				ed->modf = 1;
				
				for(i = 0;i < 0x85; i++)
				{
					if(((short*)(rom + 0x14813))[i] == ed->mapnum) 
						FixEntScroll(ed->ew.doc,i);
				}
					
				for( ; i < 0x8c; i++)
				{
					if(((short*)(rom + 0x15a64))[i] == ed->mapnum) 
						FixEntScroll(ed->ew.doc, i);
				}
					goto updmap;
			
				case 3023:
					if(j > 30)
					{
						SetDlgItemInt(win, 3023, 30, 0);
						break;
					}
					else if(j < 0)
					{
						SetDlgItemInt(win, 3023, 0, 0);
						break;
					}
				
					ed->gfxtmp = j;
					rom[ed->ew.param + (ed->ew.param >= 0x85 ? 0x15b83 : 0x15381)] = j;
					ed->ew.doc->modf = 1;
updgfx:
				
					for(i = 0; i < 0; i++) 
						Releaseblks(ed->ew.doc,ed->blocksets[i]);
				
					j = 0x6073+(ed->gfxtmp<<3);
					l = 0x5d97+(ed->gfxnum<<2);
				
					for(i = 0; i < 8 ; i++)
						ed->blocksets[i] = rom[j++];

					for(i = 3; i < 7; i++)
					{
						if(m = rom[l++]) 
							ed->blocksets[i] = m;
					}

					ed->blocksets[8] = rom[0x1011e + ed->gfxnum];
				
					for(i=0;i<9;i++) 
						Getblocks(ed->ew.doc,ed->blocksets[i]);
	
					goto updscrn;
			
				case 3041:
					if(j > 23) 
					{
						SetDlgItemInt(win, 3041, 23, 0);
						break;
					}
					else if(j < 0)
					{
						SetDlgItemInt(win, 3041, 0, 0);
						break;
					}
					
					ed->gfxnum = j;
					ed->modf = ed->hmodf = 1;
		
					goto updgfx;
			
				case 3050:
					ed->sprgfx = j;
					ed->modf = ed->hmodf = 1;
				
					l = 0x5c57 + (j << 2);
		
					for(i = 0; i < 4; i++) 
					{
						Releaseblks(ed->ew.doc, ed->blocksets[i + 11]);
				
						ed->blocksets[i+11]=rom[l+i]+0x73;
					
						Getblocks(ed->ew.doc,ed->blocksets[i+11]);
				
					}
				
					break;
updpal:
					rom = ed->ew.doc->rom;
updpal3:
	
					j = ed->palnum;
				
					goto updpal2;
			
				case 3043:
				if(j>71) {SetDlgItemInt(win,3043,71,0);break;}
				else if(j<0) {SetDlgItemInt(win,3043,0,0);break;}
				ed->palnum=j;
				if(ed->ew.param<0x8c) ed->modf=ed->hmodf=1;
updpal2:
				buf2=rom+(j<<2)+0x75460;
				i=0x1bd734+buf2[0]*90;
				Loadpal(ed,rom,i,0x21,15,6);
				Loadpal(ed,rom,i,0x89,7,1);
				Loadpal(ed,rom,0x1bd39e+buf2[1]*14,0x81,7,1);
				Loadpal(ed,rom,0x1bd4e0+buf2[2]*14,0xd1,7,1);
				Loadpal(ed,rom,0x1bd4e0+buf2[3]*14,0xe1,7,1);
				Loadpal(ed,rom,0x1bd308,0xf1,15,1);
				Loadpal(ed,rom,0x1bd648,0xdc,4,1);
				Loadpal(ed,rom,0x1bd630,0xd9,3,1);
				Loadpal(ed,rom,0x1bd218,0x91,15,4);
				Loadpal(ed,rom,0x1bd6a0,0,16,2);
				Loadpal(ed,rom,0x1bd4d2,0xe9,7,1);
				if(ed->hbuf[4]==6) Loadpal(ed,rom,0x1be22c,0x7b,2,1);
				Updpal(ed);
				break;
			case 3052:
				((short*)(rom+(ed->ew.param>=0x85?0x15af0:0x15277)))[ed->ew.param]=j;
				ed->ew.doc->modf=1;
				break;
			case 3054:
				((short*)(rom+(ed->ew.param>=0x85?0x15ae2:0x1516d)))[ed->ew.param]=j;
				ed->ew.doc->modf=1;
				break;
			} else {
				j=ed->ew.param;
				k=GetDlgItemInt(win,3002,0,0);
				if((unsigned)k>295) {ed->init=1;SetDlgItemInt(win,3002,295,0);ed->init=0;k=295;}
				((short*)(rom+(j>=0x85?0x15a64:0x14813)))[j]=k;
				i=(k&496)<<5;
				l=(k&15)<<9;
				m=GetDlgItemInt(win,3008,0,0);
				n=GetDlgItemInt(win,3010,0,0);
				if(j>=0x85) {
					((short*)(rom+0x15ac6))[j]=GetDlgItemInt(win,3004,0,0)+i;
					((short*)(rom+0x15ad4))[j]=GetDlgItemInt(win,3006,0,0)+l;
					((short*)(rom+0x15ab8))[j]=m+i;
					((short*)(rom+0x15aaa))[j]=n+l;
				} else {
					((short*)(rom+0x14f59))[j]=GetDlgItemInt(win,3004,0,0)+i;
					((short*)(rom+0x15063))[j]=GetDlgItemInt(win,3006,0,0)+l;
					((short*)(rom+0x14e4f))[j]=m+i;
					((short*)(rom+0x14d45))[j]=n+l;
				}
				SetDlgItemInt(win,3052,n+127,0);
				SetDlgItemInt(win,3054,m+119,0);
				if(wparam<3007) FixEntScroll(ed->ew.doc,j);
			}
			break;
		case 3025|(CBN_SELCHANGE<<16):
		case 3027|(CBN_SELCHANGE<<16):
		case 3039|(CBN_SELCHANGE<<16):
		case 3045|(CBN_SELCHANGE<<16):
			rom=ed->ew.doc->rom;
			j=SendMessage((HWND)lparam,CB_GETCURSEL,0,0);
			if(j==-1) break;
			switch(wparam&65535) {
			case 3025:
				rom[(ed->ew.param>=0x85?0x15bc9:0x1582e)+ed->ew.param]=mus_ofs[j];
				ed->ew.doc->modf=1;
				break;
			case 3027:
				rom[(ed->ew.param>=0x85?0x15b91:0x1548b)+ed->ew.param]=(j<<1)-2;
				ed->ew.doc->modf=1;
				goto updscrn;
			case 3039:
				ed->layering=bg2_ofs[j];
				ed->hmodf=ed->modf=1;
				goto updscrn;
			case 3045:
				ed->coll=j;
				ed->hmodf=ed->modf=1;
				break;
			}
			break;
		case 3055:
			dunged=ed;
			ShowDialog(hinstance,(LPSTR)IDD_DIALOG14,framewnd,editdungprop,0);
			break;
		case 3056:
			dunged=ed;
			ShowDialog(hinstance,(LPSTR)IDD_DIALOG15,framewnd,editroomprop,0);
			goto updpal;
		case 3057:
			hc=GetDlgItem(win,3011);
			Dungselectchg(ed,hc,0);
			ed->selchk=7;
			ed->selobj=(ed->ssize>2)?2:0;
			Dungselectchg(ed,hc,1);
			break;
		case 3058:
			hc=GetDlgItem(win,3011);
			Dungselectchg(ed,hc,0);
			ed->selchk=8;
			ed->selobj=0;
			Dungselectchg(ed,hc,1);
			break;
		case 3059:
			hc=GetDlgItem(win,3011);
			Dungselectchg(ed,hc,0);
			ed->selchk=9;
			ed->selobj=(ed->tsize>2)?2:0;
			Dungselectchg(ed,hc,1);
			break;
		case 3060:
			*ed->ebuf^=1;
			ed->modf=1;
			break;
		case 3061:
			rom=ed->ew.doc->rom;
			for(i=0;i<79;i++)
			{	
				if(((unsigned short*)(rom+0x15d8a))[i]==ed->mapnum)
					goto foundexit;
			
			}
				
			MessageBox(framewnd,"None is set.","Bad error happened",MB_OK);
			
			break;
foundexit:

			SendMessage(ed->ew.doc->editwin,4000,rom[0x15e28+i]+0x20000,0);
		}

		break;
	}

	return FALSE;

}

//dungdlgproc*******************************************

//getgbmap**********************************************

int getbgmap(OVEREDIT*ed,int a,int b) {
	int c,d;
	switch(a) {
	case 0:
		d=0;
		if(b==2) c=158; else c=151;
		break;
	case 64:
		d=0;
		c=151;
		break;
	case 128:
		d=2;
		if(b==2) c=160; else c=151;
		break;
	case 3: case 5: case 7:
		d=1;
		c=149;
		break;
	case 67: case 69: case 71:
		d=1;
		c=156;
		break;
	case 136: case 147:
		d=2;
		c=159;
		break;
	case 91:
		if(b) {
			d=1;
			c=150;
			break;
		}
	default:
		if(b) d=2; else
	case 112:
		d=0;
		c=159;
	}
	if(b || a>=128) ed->layering=d; else ed->layering=0;
	return c;
}


void loadovermap(short *b4, int m, int k, unsigned char *rom)
{
	int i,l;
	unsigned char *b2, *b3;
	if(m > 159)
		return;
	
	b2 = Uncompress(rom + romaddr(*(int*) (rom + 0x1794d + m*3)), 0, 1);
	b3 = Uncompress(rom + romaddr(*(int*) (rom + 0x17b2d + m*3)), 0, 1);
	
	for(l = 0; l < 16; l++)
	{
		for(i = 0;i < 16; i++)
		{
			*(b4++) = *(b3++) + (*(b2++) << 8);
		}

		if(!k) b4 += 16;
	}

	free(b2 - 256);
	free(b3 - 256);
}


const static int wmap_ofs[4]={0,32,2048,2080};


void Wmapselectwrite(WMAPEDIT*ed) {
	int i,j,k,l,m,n,o,p,q;
	m=ed->ew.param?32:64;
	if(ed->stselx!=ed->rectleft || ed->stsely!=ed->recttop) {
		ed->undomodf=ed->modf;
		memcpy(ed->undobuf,ed->buf,0x1000);
		if(ed->rectleft<0) i=-ed->rectleft; else i=0;
		if(ed->recttop<0) j=-ed->recttop; else j=0;
		if(ed->rectright>m) k=m; else k=ed->rectright;
		k-=ed->rectleft;
		if(ed->rectbot>m) l=m; else l=ed->rectbot;
		l-=ed->recttop;
		q=ed->rectright-ed->rectleft;
		p=j*q;
		o=(ed->recttop+j<<6)+ed->rectleft;
		for(;j<l;j++) {
			for(n=i;n<k;n++) {
				ed->buf[n+o]=ed->selbuf[n+p];
			}
			o+=64;
			p+=q;
		}
		ed->modf=1;
	}
	ed->selflag=0;
	free(ed->selbuf);
}

void Paintfloor(LMAPEDIT*ed)
{
	int i,j,k,l,m,n,o,p,q;
	short*nbuf;
	unsigned char*rom=ed->ew.doc->rom;
	m=ed->ew.param;
	nbuf=ed->nbuf;
	for(i=0x6f;i<0x1ef;i+=32) nbuf[i+1]=nbuf[i]=0xf00;
	if(ed->curfloor>=0) {
		nbuf[0x1af]=((short*)(rom+0x564e9))[ed->curfloor]&0xefff;
		nbuf[0x1b0]=0xf1d;
	} else {
		nbuf[0x1af]=0xf1c;
		nbuf[0x1b0]=((short*)(rom+0x564e9))[ed->curfloor^-1]&0xefff;
	}
	for(i=0;i<4;i++) {
		nbuf[((short*)(rom+0x56431))[i]>>1]=((short*)(rom+0x56429))[i]&0xefff;
	}
	for(j=0;j<2;j++) {
		k=((short*)(rom+0x5643d))[j]>>1;
		for(i=0;i<10;i++) {
			nbuf[(k+i)&0x7ff]=((short*)(rom+0x56439))[j]&0xefff;
		}
	}
	for(j=0;j<2;j++) {
		k=((short*)(rom+0x56445))[j]>>1;
		for(i=0;i<0x140;i+=32) {
			nbuf[(k+i)&0x7ff]=((short*)(rom+0x56441))[j]&0xefff;
		}
	}
	k=(ed->basements+ed->curfloor)*25;
	n=0x92;
	for(o=0;o<25;o+=5) {
		for(i=0;i<5;i++) {
			l=ed->rbuf[k];
			if(l==15) j=0x51; else {
				q=0;
				for(p=0;;p++) {
					j=ed->rbuf[p];
					if(j!=15) if(j==l) break; else q++;
				}
				j=ed->buf[q];
			}
			j<<=2;
			l=((short*)(rom+0x57009))[j];
			nbuf[n]=l;
			l=((short*)(rom+0x5700b))[j];
			nbuf[n+1]=l;
			l=((short*)(rom+0x5700d))[j];
			nbuf[n+32]=l;
			l=((short*)(rom+0x5700f))[j];
			nbuf[n+33]=l;
			n+=2;
			k++;
		}
		n+=54;
	}
}
void Updtmap(TMAPEDIT*ed)
{
	int i,j,k,l;
	unsigned char nb[0x6000];
	unsigned char*b=ed->buf;
	for(i=0;i<0x2000;i++) ((short*)nb)[i]=ed->back1;
	for(;i<0x3000;i++) ((short*)nb)[i]=ed->back2;
	for(;;) {
		j=(*b<<8)+b[1];
		if(j>=32768) break;
		if(j>=0x6000) j-=0x4000;
		j+=j;
		k=((b[2]&63)<<8)+b[3]+1;
		l=b[2];
		b+=4;
		if(l&64) k++;
		k>>=1;
		if(l&128) {
			if(l&64) { for(;k;k--) *(short*)(nb+j)=*(short*)b,j+=64; b+=2; }
			else for(;k;k--) *(short*)(nb+j)=*(short*)b,b+=2,j+=64;
		} else {
			if(l&64) { for(;k;k--) *(short*)(nb+j)=*(short*)b,j+=2; b+=2; }
			else for(;k;k--) *(short*)(nb+j)=*(short*)b,b+=2,j+=2;
		}
	}
	for(i=0;i<0x6000;i+=64) {
		b=(unsigned char*)(ed->nbuf+((i&0x7000)>>1)+(i&0x7c0)+((i&0x800)>>6));
		for(j=0;j<64;j+=2) *(short*)(b+j)=*(short*)(nb+i+j);
	}
}
void Gettmapsize(unsigned char*b,RECT*rc,int*s,int*t,int*u)
{
	int k,l,m,n,o,p,q,r;
	k=(*b<<8)+b[1];
	m=b[2];
	l=((m&63)<<8)+b[3]+1;
	n=(k&31)+((k&1024)>>5);
	o=((k&992)>>5)+((k&14336)>>6);
	if(!s) s=t=u=&r;
	if(m&64) l++,*s=6; else *s=l+4;
	*t=k;
	*u=l;
	l>>=1;
	if(m&128) p=n+1,q=o+l; else p=n+l,q=o+1;
	r=(p-1>>5)-(n>>5);
	if(r) q+=r,n-=(n&31),p=n+32;
	rc->left=n<<3;
	rc->top=o<<3;
	rc->right=p<<3;
	rc->bottom=q<<3;
}
short cost[256];
int rptx,rpty,rptz;
void Get3dpt(PERSPEDIT*ed,int x,int y,int z,POINT*pt)
{
	int cx,cy,cz,sx,sy,sz,a,b,c,d;
	cx=cost[ed->xrot];
	cy=cost[ed->yrot];
	cz=cost[ed->zrot];
	sx=cost[(ed->xrot+192)&255];
	sy=cost[(ed->yrot+192)&255];
	sz=cost[(ed->zrot+192)&255];
	a=cy*x-sy*z>>14; // A=X1
	b=cy*z+sy*x>>14; // B=Z1
	c=cx*b-sx*y>>14; // C=Z2
	b=cx*y+sx*b>>14; // B=Y2
	d=cz*a-sz*b>>14; // D=X3
	a=cz*b+sz*a>>14; // A=Y3
	c+=250;
//	a=cx*x-sx*y>>14;
//	b=cx*y+sx*x>>14;
//	c=cz*z-sz*b>>14;
//	b=cz*b+sz*z>>14;
//	d=cy*a-sy*c>>14;
//	a=(cy*c+sy*a>>14)+250;
	pt->x=d*ed->scalex/c+(ed->width>>1);
	pt->y=a*ed->scaley/c+(ed->height>>1);
	rptx=d;
	rpty=a;
	rptz=c;
}
/*
void Perspselchg(PERSPEDIT*ed,HWND win)
{
	RECT rc;
	POINT pt;
	int j;
	if(ed->selpt==-1) return;
	j=*(unsigned short*)(ed->buf+2+ed->objsel)-0xff8c+3*ed->selpt;
	Get3dpt(ed,ed->buf[j],-ed->buf[j+1],ed->buf[j+2],&pt);
	rc.left=pt.x-3;
	rc.right=pt.x+3;
	rc.top=pt.y-3;
	rc.bottom=pt.y+3;
	InvalidateRect(win,&rc,1);
	rc.left=0;
	rc.right=200;
	rc.top=0;
	rc.bottom=24;
	InvalidateRect(win,&rc,1);
}
*/
BOOL CALLBACK perspdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	HWND hc;
	PERSPEDIT*ed;
	int i,j;
	switch(msg) {
	case WM_INITDIALOG:
		SetWindowLong(win,DWL_USER,lparam);
		ed=(PERSPEDIT*)lparam;
		ed->xrot=ed->yrot=ed->zrot=0;
		ed->objsel=0;
		ed->tool=0;
		ed->selpt=-1;
		ed->enlarge=100;
		ed->newlen=0;
		ed->newptp=-1;
		ed->modf=0;
		memcpy(ed->buf,ed->ew.doc->rom+0x4ff8c,116);
		i=*(unsigned short*)(ed->buf+10)-0xff8c;
		j=ed->buf[7];
		for(;j;j--) i+=ed->buf[i]+2;
		ed->len=i;
		hc=GetDlgItem(win,3000);
		SetWindowLong(hc,GWL_USERDATA,lparam);
		Updatesize(hc);
		SendDlgItemMessage(win,3001,BM_SETIMAGE,IMAGE_BITMAP,(int)arrows_imgs[0]);
		SendDlgItemMessage(win,3002,BM_SETIMAGE,IMAGE_BITMAP,(int)arrows_imgs[1]);
		wsprintf(buffer,"Free: %d",116-ed->len);
		SetDlgItemText(win,3003,buffer);
		CheckDlgButton(win,3004,BST_CHECKED);
		break;
	case WM_COMMAND:
		ed=(PERSPEDIT*)GetWindowLong(win,DWL_USER);
		switch(wparam) {
		case 3001:
			ed->objsel=0;
			goto updsel;
		case 3002:
			ed->objsel=6;
			goto updsel;
		case 3004:
			ed->tool=0;
updsel:
			ed->newptp=-1;
			ed->newlen=0;
			ed->selpt=-1;
			InvalidateRect(GetDlgItem(win,3000),0,1);
			break;
		case 3005:
			ed->tool=1;
			goto updsel;
		case 3006:
			ed->tool=2;
			goto updsel;
		case 3007:
			ed->tool=3;
			goto updsel;
		}
	}
	return FALSE;
}
int tmap_ofs[]={0x64ed0,0x64e5f,0x654c0,0x65148,0x65292};
BOOL CALLBACK tmapdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	TMAPEDIT*ed;
	int i,k,l,m;
	unsigned char*rom;
	BLOCKSEL8*bs;
	HWND hc;
	HPALETTE oldpal;
	HDC hdc;
	switch(msg) {
	case WM_INITDIALOG:
		SetWindowLong(win,DWL_USER,lparam);
		ed=(TMAPEDIT*)lparam;
		ed->hpal=0;
		ed->bmih=zbmih;
		ed->anim=3;
		ed->modf=0;
		ed->sel=-1;
		ed->selbg=0;
		ed->tool=0;
		ed->withfocus=0;
		ed->mapscrollh=ed->mapscrollv=0;
		rom=ed->ew.doc->rom;
		ed->disp=3;
		ed->layering=98;
		if(!ed->ew.param) {
			ed->gfxtmp=rom[0x64207];
			ed->sprgfx=rom[0x6420c];
			ed->gfxnum=rom[0x64211];
			ed->comgfx=rom[0x6433d];
			ed->anigfx=rom[0x64223];
			ed->pal1=rom[0x145d6];
			ed->pal6=ed->pal2=rom[0x145db];
			ed->pal3=rom[0x145e8];
			ed->pal4=rom[0x145ed];
			ed->pal5=rom[0x145e3];
			i=0;
			Loadpal(ed,rom,0x1be6c8+70*ed->pal1,0x21,7,5);
			Loadpal(ed,rom,0x1be86c+42*ed->pal2,0x29,7,3);
			Loadpal(ed,rom,0x1be86c+42*ed->pal6,0x59,7,3);
			Loadpal(ed,rom,0x1bd446+14*ed->pal3,0xe9,7,1);
			Loadpal(ed,rom,0x1bd39e+14*ed->pal4,0x81,7,1);
			Loadpal(ed,rom,0x1be604+14*ed->pal5,0x71,7,1);
			Loadpal(ed,rom,0x1bd218,0x91,15,4);
			Loadpal(ed,rom,0x1bd6a0,0,16,2);
			ed->back1=ed->back2=0x1ec;
		} else {
			i=ed->ew.param+1;
			if(i==6) {
				Getblocks(ed->ew.doc,220);
				Getblocks(ed->ew.doc,221);
				Getblocks(ed->ew.doc,222);
				ed->gfxtmp=32;
				ed->gfxnum=64;
				ed->sprgfx=128;
				ed->anigfx=90;
				ed->comgfx=116;
				Loadpal(ed,rom,0x1be544,0x20,16,6);
				Loadpal(ed,rom,0x1bd6a0,0,16,2);
				Loadpal(ed,rom,0x1bd70a,0xc1,7,3);
				Loadpal(ed,rom,0x1bd642,0xd9,3,1);
				Loadpal(ed,rom,0x1bd658,0xdc,4,1);
				Loadpal(ed,rom,0x1bd344,0xf1,15,1);
				ed->layering=100;
				ed->back1=768;
				ed->back2=127;
			} else {
				ed->gfxtmp=rom[0x64dd5];
				ed->sprgfx=rom[0x6420c];
				ed->gfxnum=rom[0x64dda];
				ed->comgfx=rom[0x64dd0];
				ed->anigfx=rom[0x64223];
				ed->pal1=rom[0x64db4]>>1;
				ed->pal2=rom[0x64dc4];
				Loadpal(ed,rom,0x1bd734+180*ed->pal1,0x21,15,6);
				Loadpal(ed,rom,0x1bd734+180*ed->pal1,0x91,7,1);
				Loadpal(ed,rom,0x1be604,0x71,7,1);
				Loadpal(ed,rom,0x1bd660+32*ed->pal2,0,16,2);
				ed->back1=127;
				ed->back2=169;
			}
		}
		ed->datnum=i;
		if(i<7) {
			k=romaddr(rom[0x137d+i]+(rom[0x1386+i]<<8)+(rom[0x138f+i]<<16));
			i=0;
			for(;;) {
				if(rom[k+i]>=128) break;
				if(rom[k+i+2]&64) i+=6; else i+=((rom[k+i+2]&63)<<8)+rom[k+i+3]+5;
			}
			i++;
		} else {
			k=*(short*)(rom+tmap_ofs[i-7])+0x68000;
			if(i==7 || i==9) k++;
			switch(i) {
			case 7:
				i=*(unsigned short*)(rom+0x64ecd);
				break;
			case 8:
				i=*(unsigned short*)(rom+0x64e5c)+3;
				ed->buf=malloc(i);
				memcpy(ed->buf,rom+k,i-1);
				ed->buf[i-1]=255;
				goto loaded;
			case 9:
				i=*(unsigned short*)(rom+0x654bd);
				break;
			case 10:
				i=*(unsigned short*)(rom+0x65142)+1;
				break;
			case 11:
				i=*(unsigned short*)(rom+0x6528d)+1;
				break;
			}
		}
		ed->buf=malloc(i);
		memcpy(ed->buf,rom+k,i);
loaded:
		ed->len=i;
		for(i=16;i<256;i+=16) ed->pal[i]=ed->pal[0];
		k=0x6073+(ed->gfxtmp<<3);
		l=0x5d97+(ed->gfxnum<<2);
		for(i=0;i<8;i++) ed->blocksets[i]=rom[k++];
		for(i=3;i<7;i++) if(m=rom[l++]) ed->blocksets[i]=m;
		ed->blocksets[8]=ed->anigfx;
		ed->blocksets[9]=ed->anigfx+1;
		ed->blocksets[10]=ed->comgfx+0x73;
		k=0x5b57+(ed->sprgfx<<2);
		for(i=0;i<4;i++) ed->blocksets[i+11]=rom[k+i]+0x73;
		for(i=0;i<15;i++) Getblocks(ed->ew.doc,ed->blocksets[i]);
		if(ed->datnum!=6) Getblocks(ed->ew.doc,224);
		bs=&(ed->bs);
		Addgraphwin((DUNGEDIT*)ed,1);
		Setdispwin((DUNGEDIT*)ed);
		hdc=GetDC(win);
		hc=GetDlgItem(win,3001);
		bs->ed=(OVEREDIT*)ed;
		InitBlksel8(hc,bs,ed->hpal,hdc);
		ReleaseDC(win,hdc);
		Updtmap(ed);
		CheckDlgButton(win,3002,BST_CHECKED);
		CheckDlgButton(win,3004,BST_CHECKED);
		CheckDlgButton(win,3012,BST_CHECKED);
		CheckDlgButton(win,3013,BST_CHECKED);
		hc=GetDlgItem(win,3000);
		SetWindowLong(hc,GWL_USERDATA,(long)ed);
		Updatesize(hc);
		SetDlgItemInt(win,3008,0,0);
		break;
	case WM_DESTROY:
		ed=(TMAPEDIT*)GetWindowLong(win,DWL_USER);
		ed->ew.doc->tmaps[ed->ew.param]=0;
		Delgraphwin((DUNGEDIT*)ed);
		DeleteDC(ed->bs.bufdc);
		DeleteObject(ed->bs.bufbmp);
		for(i=0;i<15;i++) Releaseblks(ed->ew.doc,ed->blocksets[i]);
		if(ed->datnum==6) {
			Releaseblks(ed->ew.doc,220);
			Releaseblks(ed->ew.doc,221);
			Releaseblks(ed->ew.doc,222);
		} else Releaseblks(ed->ew.doc,224);
		free(ed->buf);
		free(ed);
		break;
	case 4002:
		InvalidateRect(GetDlgItem(win,3000),0,0);
		InvalidateRect(GetDlgItem(win,3001),0,0);
		break;
	case 4000:
		ed=(TMAPEDIT*)GetWindowLong(win,DWL_USER);
		i=wparam&0x3ff;
		hc=GetDlgItem(win,3001);
		Changeblk8sel(hc,&(ed->bs));
		ed->bs.sel=i;
		Changeblk8sel(hc,&(ed->bs));
		break;
	case WM_COMMAND:
		ed=(TMAPEDIT*)GetWindowLong(win,DWL_USER);
		bs=&(ed->bs);
		switch(wparam) {
		case (EN_CHANGE<<16)+3008:
			bs->flags&=0xe000;
			bs->flags|=(GetDlgItemInt(win,3008,0,0)&7)<<10;
updflag:
			if((bs->flags&0xdc00)!=bs->oldflags) {
				bs->oldflags=bs->flags&0xdc00;
updblk:
				oldpal=SelectPalette(objdc,ed->hpal,1);
				SelectPalette(bs->bufdc,ed->hpal,1);
				for(i=0;i<256;i++) Updateblk8sel(bs,i);
				SelectPalette(objdc,oldpal,1);
				SelectPalette(bs->bufdc,oldpal,1);
				InvalidateRect(GetDlgItem(win,3001),0,0);
			}
			break;
		case 3002:
			ed->selbg=0;
			ed->bs.dfl=0;
			goto updblk;
			break;
		case 3003:
			ed->selbg=1;
			ed->bs.dfl=0;
			goto updblk;
		case 3004:
			ed->tool=0;
			break;
		case 3005:
			ed->tool=1;
			break;
		case 3006:
			ed->selbg=2;
			if(ed->datnum==6) ed->bs.dfl=16;
			else ed->bs.dfl=8;
			goto updblk;
		case 3009:
			bs->flags^=0x4000;
			goto updflag;
		case 3010:
			bs->flags^=0x8000;
			goto updflag;
		case 3011:
			bs->flags^=0x2000;
			goto updflag;
		case 3012:
			ed->disp^=1;
upddisp:
			InvalidateRect(GetDlgItem(win,3000),0,1);
			break;
		case 3013:
			ed->disp^=2;
			goto upddisp;
		}
	}
	return FALSE;
}
BOOL CALLBACK lmapdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	LMAPEDIT*ed;
	unsigned char*rom;
	HWND hc;
	int i,j,k,l,m;
	switch(msg) {
	case WM_INITDIALOG:
		SetWindowLong(win,DWL_USER,lparam);
		ed=(LMAPEDIT*)lparam;
		rom=ed->ew.doc->rom;
		ed->hpal=0;
		ed->init=1;
		ed->modf=0;
		m=ed->ew.param;
		SetDlgItemInt(win,3002,ed->level=(((char)rom[0x56196+m])>>1)+1,0);
		i=((short*)(rom+0x575d9))[m];
		ed->floors=(i>>4)&15;
		ed->basements=i&15;
		ed->bg=i>>8;
		if(i&256) CheckDlgButton(win,3008,BST_CHECKED);
		if(i&512) CheckDlgButton(win,3009,BST_CHECKED);
		ed->bossroom=((short*)(rom+0x56807))[m];
		ed->bossofs=((short*)(rom+0x56e5d))[m];
		SendDlgItemMessage(win,3006,BM_SETIMAGE,IMAGE_BITMAP,(long)arrows_imgs[2]);
		SendDlgItemMessage(win,3007,BM_SETIMAGE,IMAGE_BITMAP,(long)arrows_imgs[3]);
		CheckDlgButton(win,3011,BST_CHECKED);
		ed->tool=0;
		i=25*(ed->floors+ed->basements);
		ed->rbuf=malloc(i);
		memcpy(ed->rbuf,rom+0x58000+((short*)(rom+0x57605))[m],i);
		k=0;
		for(j=0;j<i;j++) if(ed->rbuf[j]!=15) k++;
		ed->buf=malloc(k);
		memcpy(ed->buf,rom+((short*)(rom+*(short*)(rom+0x56640)+0x58000))[m]+0x58000,k);
		if(ed->bossroom==15) ed->bosspos=-1;
		else for(j=0;j<i;j++) if(ed->rbuf[j]==ed->bossroom) {ed->bosspos=j;break;}
		ed->len=k;
		ed->init=0;
		ed->gfxtmp=0x20;
		ed->disp=0;
		ed->blksel=0;
		k=0x6173;
		j=0x5e97;
		for(i=0;i<8;i++) ed->blocksets[i]=rom[k++];
		for(i=3;i<7;i++) if(l=rom[j++]) ed->blocksets[i]=l;
		k=0x5b57+((128|m)<<2);
		for(i=0;i<4;i++) ed->blocksets[i+11]=rom[k++]+0x73;
		ed->blocksets[8]=90;
		ed->blocksets[9]=91;
		ed->blocksets[10]=116;
		for(i=0;i<15;i++) Getblocks(ed->ew.doc,ed->blocksets[i]);
		ed->bmih=zbmih;
		ed->anim=0;
		Loadpal(ed,rom,0x1be544,0x20,16,6);
		Loadpal(ed,rom,0x1bd6a0,0,16,2);
		Loadpal(ed,rom,0x1bd70a,0xc1,7,3);
		Loadpal(ed,rom,0x1bd642,0xd9,3,1);
		Loadpal(ed,rom,0x1bd658,0xdc,4,1);
		Loadpal(ed,rom,0x1bd344,0xf1,15,1);
		for(i=16;i<256;i+=16) ed->pal[i]=ed->pal[0];
		ed->curfloor=0;
		ed->sel=0;
		if(!ed->floors) ed->curfloor=-1;
		hc=GetDlgItem(win,3010);
		SetWindowLong(hc,GWL_USERDATA,(int)ed);
		ed->blkscroll=0;
		hc=GetDlgItem(win,3004);
		SetWindowLong(hc,GWL_USERDATA,(int)ed);
		Updatesize(hc);
		Addgraphwin((DUNGEDIT*)ed,2);
paintfloor:
		Paintfloor(ed);
updscrn:
		hc=GetDlgItem(win,3010);
		InvalidateRect(hc,0,0);
		break;
	case WM_COMMAND:
		ed=(LMAPEDIT*)GetWindowLong(win,DWL_USER);
		if(!ed) break;
		rom=ed->ew.doc->rom;
		if(ed->init) break;
		switch(wparam) {
		case 3000:
			ed->tool=2;
			goto updscrn;
		case 3002|(EN_CHANGE<<16):
			ed->level=GetDlgItemInt(win,3002,0,0);
			ed->modf=1;
			break;
		case 3005:
			ed->disp^=1;
			InvalidateRect(GetDlgItem(win,3004),0,0);
			InvalidateRect(GetDlgItem(win,3010),0,0);
			break;
		case 3006:
			if(ed->curfloor==ed->floors-1) break;
			ed->curfloor++;
			goto paintfloor;
		case 3007:
			if(ed->curfloor==-ed->basements) break;
			ed->curfloor--;
			goto paintfloor;
		case 3008:
			ed->bg^=1;
			ed->modf=1;
			break;
		case 3009:
			ed->bg^=1;
			ed->modf=1;
			break;
		case 3011:
			ed->tool=0;
			goto updscrn;
		case 3012:
			ed->tool=1;
			goto updscrn;
		case 3013:
			if(ed->basements+ed->floors==2) {
				MessageBox(framewnd,"There must be at least two floors.","Bad error happened",MB_OK);
				break;
			}
			i=(ed->curfloor+ed->basements)*25;
			k=0;
			for(j=0;j<i;j++) if(ed->rbuf[j]!=15) k++;
			l=0;
			for(j=0;j<25;j++) if(ed->rbuf[j+i]!=15) l++;
			memcpy(ed->buf+k,ed->buf+k+l,ed->len-k-l);
			ed->len-=l;
			ed->buf=realloc(ed->buf,ed->len);
			memcpy(ed->rbuf+i,ed->rbuf+i+25,(ed->floors-ed->curfloor-1)*25);
			if(ed->bosspos>=i) ed->bosspos-=25;
			if(ed->bosspos>=i) ed->bossroom=15,ed->bosspos=-1;
			ed->rbuf=realloc(ed->rbuf,(ed->floors+ed->basements)*25);
			if(ed->curfloor>=0) ed->floors--;
			else ed->basements--;
			if(ed->curfloor>=ed->floors) ed->curfloor--;
			if(-ed->curfloor>ed->basements) ed->curfloor++;
			ed->modf=1;
			Paintfloor(ed);
			goto updscrn;
		case 3016:
			ed->curfloor++;
		case 3017:
			i=(ed->curfloor+ed->basements)*25;
			k=0;
			for(j=0;j<i;j++) if(ed->rbuf[j]!=15) k++;
			if(ed->curfloor>=0) ed->floors++;
			else ed->basements++,ed->curfloor--;
			l=(ed->floors+ed->basements)*25;
			ed->rbuf=realloc(ed->rbuf,l);
			memmove(ed->rbuf+i+25,ed->rbuf+i,l-i-25);
			if(ed->bosspos>i) ed->bosspos+=25;
			for(j=0;j<25;j++) ed->rbuf[i+j]=15;
			ed->modf=1;
			Paintfloor(ed);
			goto updscrn;
		}
		break;
	case WM_DESTROY:
		ed=(LMAPEDIT*)GetWindowLong(win,DWL_USER);
		Delgraphwin((DUNGEDIT*)ed);
		ed->ew.doc->dmaps[ed->ew.param]=0;
		for(i=0;i<15;i++) Releaseblks(ed->ew.doc,ed->blocksets[i]);
		free(ed->rbuf);
		free(ed->buf);
		free(ed);
		break;
	}
	return FALSE;
}
BOOL CALLBACK wmapdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	WMAPEDIT*ed;
	BLOCKSEL8*bs;
	unsigned char*rom;
	HWND hc,oldpal;
	HDC hdc;
	RECT rc;
	int i,j,k;
	int *b,*b2;
	static char*mapmark_str[10]={
		"Hyrule castle",
		"Village guy",
		"Sahasrahla",
		"Pendants",
		"Master sword",
		"Agahnim's tower",
		"First crystal",
		"All crystals",
		"Agahnim's other tower",
		"Flute locations"
	};
	switch(msg) {
	case WM_INITDIALOG:
		SetWindowLong(win,DWL_USER,lparam);
		ed=(WMAPEDIT*)lparam;
		Getblocks(ed->ew.doc,223);
		ed->hpal=0;
		ed->anim=0;
		ed->gfxtmp=255;
		ed->mapscrollh=0;
		ed->mapscrollv=0;
		ed->bmih=zbmih;
		rom=ed->ew.doc->rom;
		b=(int*)(rom+0x54727+(ed->ew.param<<12));
		j=ed->ew.param?1:4;
		for(k=0;k<j;k++) {
			b2=(int*)(ed->buf+wmap_ofs[k]);
			for(i=0;i<32;i++) {
				*(b2++)=*(b++);
				*(b2++)=*(b++);
				*(b2++)=*(b++);
				*(b2++)=*(b++);
				*(b2++)=*(b++);
				*(b2++)=*(b++);
				*(b2++)=*(b++);
				*(b2++)=*(b++);
				b2+=8;
			}
		}
		memcpy(ed->undobuf,ed->buf,0x1000);
		Loadpal(ed,rom,ed->ew.param?0xadc27:0xadb27,0,16,8);
		hc=GetDlgItem(win,3000);
		SetWindowLong(hc,GWL_USERDATA,(long)ed);
		Updatesize(hc);
		bs=&(ed->bs);
		hdc=GetDC(win);
		hc=GetDlgItem(win,3001);
		GetClientRect(hc,&rc);
		bs->ed=(OVEREDIT*)ed;
		bs->sel=0;
		bs->scroll=0;
		bs->flags=0;
		bs->dfl=0;
		bs->w=rc.right;
		bs->h=rc.bottom;
		bs->bufdc=CreateCompatibleDC(hdc);
		bs->bufbmp=CreateCompatibleBitmap(hdc,rc.right,rc.bottom);
		ReleaseDC(win,hdc);
		Addgraphwin((DUNGEDIT*)ed,2);
		Setdispwin((DUNGEDIT*)ed);
		SelectObject(bs->bufdc,bs->bufbmp);
		SelectObject(bs->bufdc,white_pen);
		SelectObject(bs->bufdc,black_brush);
		Rectangle(bs->bufdc,0,0,bs->w,bs->h);
		oldpal=SelectPalette(objdc,ed->hpal,1);
		SelectPalette(bs->bufdc,ed->hpal,1);
		for(i=0;i<256;i++) Updateblk8sel(bs,i);
		SelectPalette(objdc,oldpal,1);
		SelectPalette(bs->bufdc,oldpal,1);
		SetWindowLong(hc,GWL_USERDATA,(int)bs);
		Updatesize(hc);
		ed->tool=1;
		ed->dtool=0;
		ed->selflag=0;
		ed->modf=0;
		ed->undomodf=0;
		ed->selbuf=0;
		ed->marknum=0;
		CheckDlgButton(win,3003,BST_CHECKED);
		hc=GetDlgItem(win,3005);
		for(i=0;i<10;i++) SendMessage(hc,CB_ADDSTRING,0,(long)mapmark_str[i]);
		SendMessage(hc,CB_SETCURSEL,0,0);
		break;
	case 4002:
		InvalidateRect(GetDlgItem(win,3000),0,0);
		InvalidateRect(GetDlgItem(win,3001),0,0);
		break;
	case WM_DESTROY:
		ed=(WMAPEDIT*)GetWindowLong(win,DWL_USER);
		ed->ew.doc->wmaps[ed->ew.param]=0;
		Delgraphwin((DUNGEDIT*)ed);
		Releaseblks(ed->ew.doc,223);
		DeleteDC(ed->bs.bufdc);
		DeleteObject(ed->bs.bufbmp);
		free(ed);
		break;
	case 4000:
		ed=(WMAPEDIT*)GetWindowLong(win,DWL_USER);
		j=wparam;
		if(j<0) j=0;
		if(j>0xff) j=0xff;
		hc=GetDlgItem(win,3001);
		Changeblk8sel(hc,&(ed->bs));
		ed->bs.sel=j;
		Changeblk8sel(hc,&(ed->bs));
		break;
	case WM_COMMAND:
		ed=(WMAPEDIT*)GetWindowLong(win,DWL_USER);
		j=ed->tool;
		switch(wparam) {
		case 3002:
			ed->tool=0;
			break;
		case 3003:
			ed->tool=1;
			break;
		case 3004:
			ed->tool=2;
			break;
		case 3005|(CBN_SELCHANGE<<16):
			ed->marknum=SendMessage((HWND)lparam,CB_GETCURSEL,0,0);
			if(j==4) break;
updscrn:
			j=0;
			InvalidateRect(GetDlgItem(win,3000),0,0);
			break;
		case 3006:
			ed->tool=3;
			ed->selmark=0;
			break;
		case 3008:
			ed->tool=4;
			goto updscrn;
		case 3007:
			if(ed->selflag) Wmapselectwrite(ed);
			b2=malloc(0x1000);
			memcpy(b2,ed->buf,0x1000);
			memcpy(ed->buf,ed->undobuf,0x1000);
			memcpy(ed->undobuf,b2,0x1000);
			free(b2);
			i=ed->undomodf;
			ed->undomodf=ed->modf;
			ed->modf=i;
			goto updscrn;
		}
		if(j==4 && ed->tool!=4) goto updscrn;
	}
	return FALSE;
}
const static int blkx[16]={0,8,0,8,16,24,16,24,0,8,0,8,16,24,16,24};
const static int blky[16]={0,0,8,8,0,0,8,8,16,16,24,24,16,16,24,24};
int Keyscroll(HWND win,int wparam,int sc,int page,int scdir,int size,int size2)
{
	int i;
	switch(wparam) {
	case VK_UP:
		i=SB_LINEUP;
		break;
	case VK_DOWN:
		i=SB_LINEDOWN;
		break;
	case VK_PRIOR:
		i=SB_PAGEUP;
		break;
	case VK_NEXT:
		i=SB_PAGEDOWN;
		break;
	case VK_HOME:
		i=SB_TOP;
		break;
	case VK_END:
		i=SB_BOTTOM;
		break;
	default:
		return sc;
	}
	return Handlescroll(win,i,sc,page,scdir,size,size2);
}
int CALLBACK blk16search(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	OVEREDIT*ed;
	PAINTSTRUCT ps;
	HDC hdc,oldbrush,oldobj2,oldpal;
	RECT rc;
	SCROLLINFO si;
	int i,j,k,l,m;
	unsigned short*o;
	unsigned char*rom;
	switch(msg) {
	case WM_GETDLGCODE:
		return DLGC_WANTCHARS|DLGC_WANTARROWS;
	case WM_KEYDOWN:
		ed=(OVEREDIT*)GetWindowLong(win,GWL_USERDATA);
//		if(wparam>=65 && wparam<71) { wparam-=55; goto typedigit; }
		if(wparam>=48 && wparam<58) {
			wparam-=48;
typedigit:
			ed->schtyped/*<<=4*/*=10;
			ed->schtyped+=wparam;
			ed->schtyped%=5000;
			ed->schscroll=Handlescroll(win,SB_THUMBPOSITION|(ed->schtyped<<16),ed->schscroll,ed->schpage,SB_VERT,0xea8,16);
		} else ed->schscroll=Keyscroll(win,wparam,ed->schscroll,ed->schpage,SB_VERT,0xea8,16);
		break;
	case WM_SIZE:
		ed=(OVEREDIT*)GetWindowLong(win,GWL_USERDATA);
		if(!ed) break;
		si.cbSize=sizeof(si);
		si.fMask=SIF_RANGE|SIF_PAGE;
		si.nMin=0;
		si.nMax=0xea8;
		si.nPage=lparam>>20;
		ed->schpage=si.nPage;
		SetScrollInfo(win,SB_VERT,&si,1);
		ed->schscroll=Handlescroll(win,-1,ed->schscroll,ed->schpage,SB_VERT,0xea8,16);
		break;
	case WM_VSCROLL:
		ed=(OVEREDIT*)GetWindowLong(win,GWL_USERDATA);
		ed->schscroll=Handlescroll(win,wparam,ed->schscroll,ed->schpage,SB_VERT,0xea8,16);
		break;
	case WM_PAINT:
		ed=(OVEREDIT*)GetWindowLong(win,GWL_USERDATA);
		if(!ed) break;
		hdc=BeginPaint(win,&ps);
		oldpal=SelectPalette(hdc,ed->hpal,1);
		RealizePalette(hdc);
		i=(ps.rcPaint.top>>4);
		j=(ps.rcPaint.bottom+15>>4);
		FillRect(hdc,&(ps.rcPaint),black_brush);
		m=i+ed->schscroll;
		rom=ed->ew.doc->rom;
		oldbrush=SelectObject(hdc,trk_font);
		oldobj2=SelectObject(hdc,white_pen);
		SetTextColor(hdc,0xffffff);
		SetBkColor(hdc,0);
		for(;i<j;i+=2) {
			for(k=0;k<1024;k++) drawbuf[k]=0;
			for(k=0;k<2;k++) {
				if(m>=0xea8) break;
				o=(unsigned short*)(rom+0x78000+(m<<3));
				for(l=0;l<4;l++)
					Drawblock(ed,blkx[l]+8,blky[l]+(k<<4),*(o++),0);
				rc.left=4;
				rc.right=20;
				rc.top=(i+k<<4)+2;
				rc.bottom=rc.top+12;
				DrawFrameControl(hdc,&rc,DFC_BUTTON,((ed->selsch[m>>3]&(1<<(m&7)))?(DFCS_BUTTONCHECK|DFCS_CHECKED):DFCS_BUTTONCHECK)+((ed->schpush==m)?DFCS_PUSHED:0));
				rc.left=24;
				rc.right=40;
				DrawFrameControl(hdc,&rc,DFC_BUTTON,((ed->selsch[0x1d5+(m>>3)]&(1<<(m&7)))?(DFCS_BUTTONCHECK|DFCS_CHECKED):DFCS_BUTTONCHECK)+((ed->schpush==m+0xea8)?DFCS_PUSHED:0));
				rc.left=44;
				rc.right=60;
				DrawFrameControl(hdc,&rc,DFC_BUTTON,((ed->selsch[0x3aa+(m>>3)]&(1<<(m&7)))?(DFCS_BUTTONCHECK|DFCS_CHECKED):DFCS_BUTTONCHECK)+((ed->schpush==m+0x1d50)?DFCS_PUSHED:0));
				wsprintf(buffer,"%04d",m);
				TextOut(hdc,64,rc.top,buffer,4);
				m++;
			}
			k=i<<4;
			Paintblocks(&(ps.rcPaint),hdc,100,k,(DUNGEDIT*)ed);
			MoveToEx(hdc,108,k,0);
			LineTo(hdc,124,k);
			MoveToEx(hdc,108,k+16,0);
			LineTo(hdc,124,k+16);
		}
		SelectObject(hdc,oldbrush);
		SelectObject(hdc,oldobj2);
		SelectPalette(hdc,oldpal,1);
		EndPaint(win,&ps);
		break;
	case WM_LBUTTONDOWN:
		SetFocus(win);
		ed=(OVEREDIT*)GetWindowLong(win,GWL_USERDATA);
		i=(short)lparam;
		j=lparam>>16;
		k=(j>>4)+ed->schscroll;
		if(k<0 || k>=0xea8) break;
		if(ed->schpush!=-1)
			InvalidateRect(win,&(ed->schrc),0);
		if((j&15)>=2 && (j&15)<14) {
			if(i>4 && i<20) j=0,rc.left=4;
			else if(i>24 && i<40) j=0xea8,rc.left=24;
			else if(i>44 && i<60) j=0x1d50,rc.left=44;
			else break;
			ed->schpush=k+j;
			rc.right=rc.left+16;
			rc.top=(k-ed->schscroll<<4)+2;
			rc.bottom=rc.top+12;
			ed->schrc=rc;
			InvalidateRect(win,&rc,0);
			SetCapture(win);
		}
		break;
	case WM_LBUTTONUP:
		ed=(OVEREDIT*)GetWindowLong(win,GWL_USERDATA);
		if(ed->schpush!=-1) {
			i=(short)lparam;
			j=lparam>>16;
			rc=ed->schrc;
			if(i>=rc.left && i<rc.right && j>=rc.top && j<rc.bottom)
			ed->selsch[ed->schpush>>3]^=(1<<(ed->schpush&7));
			ed->schpush=-1;
			InvalidateRect(win,&(ed->schrc),0);
			ReleaseCapture();
		}
		break;
	default:
		return DefWindowProc(win,msg,wparam,lparam);
	}
}
BOOL CALLBACK findblks(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	OVEREDIT*ed;
	HWND hc;
	HANDLE h;
	static OPENFILENAME ofn;
	static char blkname[MAX_PATH]="findblks.dat";
	int i,j;
	switch(msg) {
	case WM_INITDIALOG:
		SetWindowLong(win,DWL_USER,lparam);
		ed=(OVEREDIT*)lparam;
		ed->schpush=-1;
		ed->schtyped=0;
		hc=GetDlgItem(win,IDC_CUSTOM1);
		SetWindowLong(hc,GWL_USERDATA,lparam);
		Updatesize(hc);
		break;
	case WM_COMMAND:
		ed=(OVEREDIT*)GetWindowLong(win,DWL_USER);
		switch(wparam) {
		case IDC_BUTTON1:
			for(i=0x1d5;i<0x3aa;i++) ed->selsch[i]=255;
upddisp:
			InvalidateRect(GetDlgItem(win,IDC_CUSTOM1),0,0);
			break;
		case IDC_BUTTON2:
			for(i=0x1d5;i<0x3aa;i++) ed->selsch[i]=0;
			goto upddisp;
		case IDC_BUTTON3:
			for(i=0;i<0x1d5;i++) ed->selsch[i]=0;
			goto upddisp;
		case IDC_BUTTON4:
			for(i=0x3aa;i<0x57f;i++) ed->selsch[i]=0;
			goto upddisp;
		case IDC_BUTTON5:
			for(i=0x3aa;i<0x57f;i++) ed->selsch[i]=255;
			goto upddisp;
		case IDC_BUTTON6:
			ofn.lStructSize=sizeof(ofn);
			ofn.hwndOwner=win;
			ofn.hInstance=hinstance;
			ofn.lpstrFilter="All files\0*.*\0";
			ofn.lpstrCustomFilter=0;
			ofn.nFilterIndex=1;
			ofn.lpstrFile=blkname;
			ofn.nMaxFile=MAX_PATH;
			ofn.lpstrFileTitle=0;
			ofn.lpstrInitialDir=0;
			ofn.lpstrTitle="Load block filter";
			ofn.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
			ofn.lpstrDefExt=0;
			ofn.lpfnHook=0;
			
			if(!GetOpenFileName(&ofn))
				break;
			
			h = CreateFile(ofn.lpstrFile,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
			
			ReadFile(h, ed->selsch,1408, &i, 0);
			CloseHandle(h);
			goto upddisp;
		case IDC_BUTTON7:
			ofn.lStructSize=sizeof(ofn);
			ofn.hwndOwner=win;
			ofn.hInstance=hinstance;
			ofn.lpstrFilter="All files\0*.*\0";
			ofn.lpstrCustomFilter=0;
			ofn.nFilterIndex=1;
			ofn.lpstrFile=blkname;
			ofn.nMaxFile=MAX_PATH;
			ofn.lpstrFileTitle=0;
			ofn.lpstrInitialDir=0;
			ofn.lpstrTitle="Save block filter";
			ofn.Flags=OFN_OVERWRITEPROMPT|OFN_PATHMUSTEXIST|OFN_HIDEREADONLY;
			ofn.lpstrDefExt=0;
			ofn.lpfnHook=0;
			if(!GetSaveFileName(&ofn)) break;
			h=CreateFile(ofn.lpstrFile,GENERIC_WRITE,0,0,CREATE_ALWAYS,FILE_FLAG_SEQUENTIAL_SCAN,0);
			WriteFile(h,ed->selsch,1408,&i,0);
			CloseHandle(h);
			break;
		case IDOK:
			j=0;
			for(i=0;i<0xea8;i++) {
				if(ed->selsch[i>>3]&(1<<(i&7))) {
					if(j==4) {
						MessageBox(framewnd,"Please check only up to 4 \"Yes\" boxes.","Bad error happened",MB_OK);
showpos:
						ed->schscroll=Handlescroll(GetDlgItem(win,IDC_CUSTOM1),SB_THUMBPOSITION+(i-(ed->schpage>>1)<<16),ed->schscroll,ed->schpage,SB_VERT,0xea8,16);
						goto brk;
					}
					if(ed->selsch[i+0xea8>>3]&(1<<(i&7))) {
						MessageBox(framewnd,"You can't require and disallow a block at the same time.","Bad error happened",MB_OK);
						goto showpos;
					}
					ed->schyes[j++]=i;
				}
			}
			for(;j<4;j++) ed->schyes[j]=-1;
			EndDialog(win,1);
brk:
			break;
		case IDCANCEL:
			EndDialog(win,0);
		}
		return 1;
	}
	return 0;
}
void Getblock32(unsigned char*rom,int m,short*l)
{
	int n;
	if(m==-1) { l[0]=l[1]=l[2]=l[3]=0xdc4; return; }
	n=(m>>2)*6;
	switch(m&3) {
	case 0:
		l[0]=rom[0x18000+n]|(rom[0x18004+n]>>4)<<8;
		l[1]=rom[0x1b400+n]|(rom[0x1b404+n]>>4)<<8;
		l[2]=rom[0x20000+n]|(rom[0x20004+n]>>4)<<8;
		l[3]=rom[0x23400+n]|(rom[0x23404+n]>>4)<<8;
		break;
	case 1:
		l[0]=rom[0x18001+n]|(rom[0x18004+n]&15)<<8;
		l[1]=rom[0x1b401+n]|(rom[0x1b404+n]&15)<<8;
		l[2]=rom[0x20001+n]|(rom[0x20004+n]&15)<<8;
		l[3]=rom[0x23401+n]|(rom[0x23404+n]&15)<<8;
		break;
	case 2:
		l[0]=rom[0x18002+n]|(rom[0x18005+n]>>4)<<8;
		l[1]=rom[0x1b402+n]|(rom[0x1b405+n]>>4)<<8;
		l[2]=rom[0x20002+n]|(rom[0x20005+n]>>4)<<8;
		l[3]=rom[0x23402+n]|(rom[0x23405+n]>>4)<<8;
		break;
	case 3:
		l[0]=rom[0x18003+n]|(rom[0x18005+n]&15)<<8;
		l[1]=rom[0x1b403+n]|(rom[0x1b405+n]&15)<<8;
		l[2]=rom[0x20003+n]|(rom[0x20005+n]&15)<<8;
		l[3]=rom[0x23403+n]|(rom[0x23405+n]&15)<<8;
		break;
	}
}
const char*sprset_str[]={
	"Beginning","First part","Second part"
};

//LoadOverlays*******************************************

void LoadOverlays(FDOC *doc)
{
	int i,k,l,m,bd,bs,o,p;

	unsigned short *b;
	unsigned char *rom = doc->rom;
	
	if(doc->o_loaded) 
		return;
	
	b = malloc(1024);
	bd = 129;
	bs = 512;

	*(int*)doc->o_enb	= *(int*)(doc->o_enb+4)
						= *(int*)(doc->o_enb+8) 
						= *(int*)(doc->o_enb+12)
						= 0;
	
	for(m = 0; m < 128; m++) 
	{
		i=0x78000+((short*)(rom+0x77664))[m];
		b[m]=bd;
	
		if(i >= doc->oolend) 
			continue;
		
		for(o = 0;o < m; o++) 
		{
			if(0x78000 + ((short*) (rom + 0x77664))[o] == i)
			{
				b[m] = b[o];
				
				goto okovl;
			}
		}
		
		k = -1;
		p = -1;
		
		for(;;) 
		{
			if(bd > bs-4) 
			{
				bs += 512;
				b = realloc(b,bs<<1);
			}
			
			switch( rom[i++] ) 
			{
			case 0x1a:
				k++;
				
				break;
			
			case 0x4c:
				i = 0x78000 + *(short*) (rom+i);
				
				break;
			
			case 0x60:
				goto okovl;
			
			case 0x8d:
				l = *(short*) (rom + i);
				i += 2;
settile:
				if(k != p) 
				{
					if(p >= 0) 
						b[bd++] = 0xffff;
					
					p = k;
					b[bd++] = k;
				}

				switch(l >> 13) 
				{
				case 1:
					b[bd++] = (l & 8191) >> 1;
					
					break;
				
				case 2:
					b[bd++] = ((l & 2047) >> 1) | 4096;
					
					break;
				
				default:
					goto badovl;
				}
				
				break;
			
			case 0x9d:
				l = *(short*)(rom+i)+o;
				i += 2;
				
				goto settile;
			
			case 0x8f:
				l = *(short*)(rom+i);
				
				if(rom[i + 2] != 126) 
					goto badovl;
				
				i += 3;
				
				goto settile;
			
			case 0x9f:
				l = *(short*) (rom + i) + o;
				
				if(rom[i + 2] != 126) 
					goto badovl;
				
				i += 3;
				
				goto settile;
			
			case 0xa2:
				o = *(short*)(rom+i);
				i += 2;
				
				break;
			
			case 0xa9:
				k = *(short*) (rom+i);
				i += 2;
				
				break;
			
			case 0xea:
				break;
			
			default:
badovl:
				free(b);
				doc->o_loaded=2;return;
			}

			if(bd > b[m] + 0x2800) 
				goto badovl;
		}
	
		goto badovl;
okovl:
		b[bd++] = 0xfffe;
		doc->o_enb[m >> 3] |= 1 << (m & 7);
	}
	
	b = realloc(b,bd << 1);
	
	doc->ovlbuf = b;
	b[128] = bd;
	doc->o_loaded = 1;
}

//LoadOverlays************************************

//SaveOverlays************************************

void SaveOverlays(FDOC *doc)
{
	int i, // counter variable
		j = 0,
		k, // another counter variable
		l,
		m,
		n,
		o,
		p,
		q = 0,
		r = 0;
	
	unsigned short *b = doc->ovlbuf;
	unsigned char *rom = doc->rom;
	
	char buf[4096];
	
	if(doc->o_loaded != 1) // if the overlays have not been loaded, exit the routine. 
		return;
	
	m = 0;
	o = 0;

	for(i = 0; i < 128; i++) 
	{
		for(k = 0; k < i; k++) 
		{	
			// k = 0,1,2,...,i
			// if b[k] is the same as the last one in the list (b[i]), copy it in the rom
			// as well.
			if(b[k] == b[i]) 
			{
				( (short*) ( rom + 0x77664 ) )[i] = ((short*) ( rom + 0x77664 ))[k];
				
				goto nextovl; // skip to the next overlay. i.e. increment i.
			}		
		}
		
		// if none of them match, take i and make k into the value of b[i].
		k = b[i];
		
		( (short*) ( rom + 0x77664 ) )[i] = j + 0xf764;
		
		if( !( doc->o_enb[i >> 3] & (1 << (i & 7) ) )) 
			continue; // beats me. seems like loop around if certain overlays are not enabled.
		
		if(b[k] == 0xfffe && m) // convert m to the next lower even integer. 
		{
			((short*) ( rom + 0x77664 ))[i] = m + 0xf864;
			
			continue;
		}
		
		p = 0;

		k = b[i];
		
		n = 0xfffe;
		
		for(;;) 
		{
			l = b[k++];
		
			if(l >= 0xfffe) 
				break;
			
			if(l == 0x918) 
				q = j;
			
			if(l - n == 1) 
			{
				if(n == p + 0x918) 
					p++; 
				else 
					p = 0;
				
				buf[j++] = 0x1a;
			} 
			else 
			{
				p = 0;
				
				if(l-n == 2) 
				{
					*(short*) (buf + j) = 0x1a1a;
					j += 2;
				}
				else 
				{
					buf[j] = 0xa9;
					*(short*) (buf + j + 1) = l;
					j += 3;
				}
			}
			
			n = l;
			
			for(;;) 
			{
				if(j >= 0x89c) 
				{
error:
					MessageBox(framewnd,"Not enough room for overlays","Bad error happened",MB_OK);
					return;
				}
				
				l = b[k++];
				
				if(l >= 0xfffe) 
					break;
				
				if(!p) 
					r = l; 
				else if( map16ofs[p] != l-r)
					p = 0;
				
				buf[j] = 0x8d;
				
				*(short*) (buf + j + 1) = (l << 1) + 0x2000;
				j += 3;
			}
			
			if(l == 0xfffe) 
				break;
		}

		if(p == 3)
		{
			j = q;
			buf[j] = 0xa2;
			
			*(short*) (buf + j + 1) = r << 1;
			if(o) 
			{
				buf[j + 3] = 0x4c;
			
				*(short*) (buf + j + 4) = o + 0xf764;
				j += 6;
			} 
			else 
			{
				j += 3;
				o = j;
				
				*(int*) (buf + j) = 0x9d0918a9;
				*(int*) (buf + j + 4) = 0x9d1a2000;
				*(int*) (buf + j + 8) = 0x9d1a2002;
				*(int*) (buf + j + 12) = 0x9d1a2080;
				*(int*) (buf + j + 16) = 0x602082;
				
				j += 19;
			}
		} 
		else 
			buf[j++] = 0x60;
		
		if(!m) 
			m = j;
		
		if(j >= 0x89c) 
			goto error;
nextovl:;
	}

	memcpy(rom + 0x77764, buf, j);
	
	doc->oolend = j + 0x77764;
	doc->o_modf = 0;
}

// SaveOverlays**********************************

// Unloadovl***************************************

void Unloadovl(FDOC *doc)
{
	if(doc->o_loaded == 1) // if the overlays are loaded...
		free(doc->ovlbuf); // free the overlay buffer.
	
	doc->o_loaded = 0; // the overlays are regarded as not being loaded. 
}

//Unloadovl******************************************

//loadoverovl***********************************

int loadoverovl(FDOC*doc,short*buf,int mapnum)
{
	unsigned short*b;
	int i,k,l;
	LoadOverlays(doc);
	if(doc->o_loaded!=1) return 0;
	b=doc->ovlbuf;
	memset(buf,-1,0x2800);
	if(!(doc->o_enb[mapnum>>3]&(1<<(mapnum&7)))) return 0;
	i=b[mapnum];
	for(;;) {
		k=b[i++];
		if(k>=0xfffe) break;
		for(;;) {
			l=b[i++];
			if(l>=0xfffe) break;
			buf[l]=k;
		}
		if(l==0xfffe) break;
	}
	return 1;
}
void Changeblk16sel(HWND win,BLOCKSEL16*ed)
{
	RECT rc,rc2;
	int i=ed->sel-(ed->scroll<<4);
	GetClientRect(win,&rc);
	rc2.left=(rc.right>>1)-64+((i&7)<<4);
	rc2.right=rc2.left+16;
	rc2.top=(i&0xfff8)<<1;
	rc2.bottom=rc2.top+16;
	InvalidateRect(win,&rc2,0);
}
void SetBS16(BLOCKSEL16*bs,int i,HWND hc)
{
	int j;
	if(i<0) i=0;
	if(i>0xea7) i=0xea7;
	j=bs->scroll<<4;
	if(i<j) j=i;
	if(i>=j+(bs->page<<4)) j=i-(bs->page<<4)+16;
	SendMessage(hc,WM_VSCROLL,SB_THUMBPOSITION|((j>>4)<<16),SB_VERT);
	Changeblk16sel(hc,bs);
	bs->sel=i;
	Changeblk16sel(hc,bs);
}


BOOL CALLBACK editvarious(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	static unsigned char *rom;
	static EDITCOMMON ec;
	static int gfxnum,sprgfx,palnum;
	static int init = 0;
	static BLOCKSEL8 bs;
	
	unsigned char*buf2;

	static int *const set_ofs[4]={&ec.gfxtmp,&gfxnum,&sprgfx,&palnum};
	
	const static unsigned set_max[4]={37,82,144,72};
	const static int ctl_ids[4]={3004,3012,3016,3020};
	const static int stuff_ofs[4]={0x54b7,0x51d3,0x4f8f,0x74894};
	unsigned i;
	int j,k,l;
	static HDC hdc;
	static HWND hc;
	switch(msg) {
	case WM_INITDIALOG:
		memset(&ec,0,sizeof(ec));
		rom=(ec.ew.doc=(FDOC*)lparam)->rom;
		ec.bmih=zbmih;
		ec.gfxtmp=0;
		ec.anim=3;
		gfxnum=0;
		init=1;
		SetDlgItemInt(win,3000,0,0);
		SetDlgItemInt(win,3001,0,0);
		SetDlgItemInt(win,3002,0,0);
		SetDlgItemInt(win,3003,0,0);
		init=0;
		Addgraphwin((void*)&ec,1);
		bs.ed=(void*)&ec;
		hdc=GetDC(win);
		InitBlksel8(hc=GetDlgItem(win,IDC_CUSTOM3),&bs,ec.hpal,hdc);
		ReleaseDC(win,hdc);
		Setdispwin((void*)&ec);
loadnewgfx:

		for(i = 0; i < 8; i++) 
			ec.blocksets[i] = rom[0x6073 + i + (ec.gfxtmp << 3)];
		
		for(i = 0; i < 4; i++)
		{
			if( (j = rom[0x5d97 + i + (gfxnum << 2)]) != 0) 
				ec.blocksets[i + 3] = j;
		}

		for(i = 0; i < 8; i++) 
			Getblocks(ec.ew.doc, ec.blocksets[i]);
updscrn:

		if(init < 2) 
		{
			for(i = 0; i < 256; i++) 
				Updateblk8sel(&bs, i);
			
			InvalidateRect(hc, 0, 0);
		}

		break;

	case WM_DESTROY:
	
		DeleteDC(bs.bufdc);
		DeleteObject(bs.bufbmp);
		Delgraphwin( (void*) &ec);

		break;
	
	case 4000:
	
		i = wparam & 0x3ff;

		Changeblk8sel(hc,&bs);

		bs.sel = i;
	
		Changeblk8sel(hc,&bs);
		
		break;

	case WM_COMMAND:
	
		if(wparam == IDCANCEL)
		{
			EndDialog(win, 0);
		
			break;
		}

		if((wparam>>16)!=EN_CHANGE) break;
		wparam&=65535;
		i=GetDlgItemInt(win,wparam,0,0);
		if(wparam==IDC_EDIT1) {
			bs.flags=(i&7)<<10;
			goto updscrn;
		}
		if(wparam<3004) {
			if(i>=set_max[wparam-3000]) {SetDlgItemInt(win,wparam,set_max[wparam-3000]-1,0);break;}
			*set_ofs[wparam-3000]=i;
			j=3;
			if(wparam==3000) j=7,i<<=1;
			k=ctl_ids[wparam-3000]+j;
			l=k+(i<<2);
			init++;
			for(;j>=0;j--) {
				SetDlgItemInt(win,k,rom[stuff_ofs[wparam-3000]+l],0);
				k--;
				l--;
			}
			init--;
			break;
		}
		if(wparam<3020) if(i>219) {SetDlgItemInt(win,wparam,219,0);break;}
		if(wparam<3012) {
			if(init>1) break;
			if(!init) rom[0x54b7+(ec.gfxtmp<<3)+wparam]=i,ec.ew.doc->modf=1;
freegfx:
			for(i=0;i<8;i++) Releaseblks(ec.ew.doc,ec.blocksets[i]);
			goto loadnewgfx;
		}
		if(wparam<3016) {
			if(init>1) break;
			if(!init) rom[0x51d3+(gfxnum<<2)+wparam]=i,ec.ew.doc->modf=1;
			goto freegfx;
		}
		if(wparam<3020) {
			if(!init) rom[0x4f8f+(sprgfx<<2)+wparam]=i,ec.ew.doc->modf=1;
			if(init<2) Releaseblks(ec.ew.doc,ec.blocksets[wparam-3005]);
			i+=115;
			ec.blocksets[wparam-3005]=i;
			Getblocks(ec.ew.doc,i);
			if(init>1 && wparam!=3016) break;
			goto updscrn;
		}
		if(wparam<3024) {
			if(init>1 && wparam!=3020) break;
			if(!init) rom[0x74894+(palnum<<2)+wparam]=i,ec.ew.doc->modf=1;
			buf2=rom+(palnum<<2)+0x75460;
			if(palnum<41) {
				i=0x1bd734+buf2[0]*90;
				Loadpal(&ec,rom,i,0x21,15,6);
				Loadpal(&ec,rom,i,0x89,7,1);
				Loadpal(&ec,rom,0x1bd39e+buf2[1]*14,0x81,7,1);
				Loadpal(&ec,rom,0x1bd4e0+buf2[2]*14,0xd1,7,1);
				Loadpal(&ec,rom,0x1bd4e0+buf2[3]*14,0xe1,7,1);
			} else {
				if(buf2[0]<128) Loadpal(&ec,rom,0x1be86c+(((unsigned short*)(rom+0xdec13))[buf2[0]]),0x29,7,3);
				if(buf2[1]<128) Loadpal(&ec,rom,0x1be86c+(((unsigned short*)(rom+0xdec13))[buf2[1]]),0x59,7,3);
				if(buf2[2]<128) Loadpal(&ec,rom,0x1be604+(((unsigned char*)(rom+0xdebc6))[buf2[2]]),0x71,7,1);
			}
			if(!ec.hpal) goto updscrn;
		}
	}
	return 0;
}
BOOL CALLBACK editbosslocs(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	static int num;
	static FDOC*doc;
	static unsigned char*rom;
	int i;
	switch(msg) {
	case WM_INITDIALOG:
		num=lparam;
		doc=activedoc;
		rom=doc->rom;
		SetDlgItemInt(win,IDC_EDIT1,i=((short*)(rom+0x10954))[num],0);
		SetDlgItemInt(win,IDC_EDIT2,rom[0x792d+num]+(i&256),0);
		SetDlgItemInt(win,IDC_EDIT3,rom[0x7939+num]+(i&256),0);
		break;
	case WM_COMMAND:
		switch(wparam) {
		case IDOK:
			((short*)(rom+0x10954))[num]=GetDlgItemInt(win,IDC_EDIT1,0,0);
			rom[0x792d+num]=GetDlgItemInt(win,IDC_EDIT2,0,0);
			rom[0x7939+num]=GetDlgItemInt(win,IDC_EDIT3,0,0);
			doc->modf=1;
		case IDCANCEL:
			EndDialog(win,0);
		}
	}
	return FALSE;
}
void Blkedit8load(BLKEDIT8*ed)
{
	int i,j,k,l,m,n=0;
	int*b,*c;
	if(ed->blknum==260) l=0,n=24;
	else if(ed->blknum==259) l=0,n=8;
	else if(ed->blknum>=256) l=ed->blknum-256<<7,n=16;
	else if(ed->blknum<8) l=ed->blknum<<6;
	else if(ed->blknum==10) l=0x240;
	else if(ed->blknum>=32) l=ed->blknum-31<<16;
	else if(ed->blknum>=15) l=(ed->blknum-5)<<6;
	else l=ed->blknum+1<<6,n=4;
	if(ed->oed->gfxtmp==0xff) m=-1; else if(n==24) m=0x0f0f0f0f; else if(n>=8) m=0x3030303; else m=0x7070707;
	for(i=ed->size-16;i>=0;i-=16) {
		for(j=0;j<128;j+=8) {
			Drawblock(ed->oed,0,24,(j>>3)+i+l,n);
			b=(int*)(ed->buf+(ed->size-16-i<<6)+j);
			c=(int*)drawbuf;
			for(k=0;k<8;k++) {
				b[0]=c[0]&m;
				b[1]=c[1]&m;
				b+=32;
				c+=8;
			}
		}
	}
}

BOOL CALLBACK blockdlgproc(HWND win,UINT msg,WPARAM wparam,LPARAM lparam)
{
	BLKEDIT8 *ed;
	OVEREDIT *oed;
	ZBLOCKS *blk;
	FDOC *doc;
	HWND hc, hc2;
	RECT rc;
	
	unsigned char *b, *b2;
	
	HGLOBAL hgl;
	
	int i, j, k, l, m[2], n, o[8];
	
	BITMAPINFOHEADER bmih;
	
	switch(msg) 
	{
	case WM_QUERYNEWPALETTE:
	
		ed = (BLKEDIT8*) GetWindowLong(win, DWL_USER);
		Setpalette(win, ed->oed->hpal);
		
		break;
	
	case WM_PALETTECHANGED:
	
		InvalidateRect(GetDlgItem(win, IDC_CUSTOM1), 0, 0);
		InvalidateRect(GetDlgItem(win, IDC_CUSTOM2), 0, 0);
	
		break;

	case WM_INITDIALOG:
	
		ed = malloc(sizeof(BLKEDIT8));
		SetWindowLong(win, DWL_USER, (int) ed);
	
		oed = (OVEREDIT*) *(int*) lparam;
		hc = GetDlgItem(win, IDC_CUSTOM1);
		SetWindowLong(hc,GWL_USERDATA,(long) ed);
	
		i = ((short*) lparam)[2];
		ed->oed = oed;
		
		if(i >= 145 && i < 147)
			i += 111;
		
		if(i == 260)
			ed->size = 896;

		else if(i == 259) 
			ed->size = 256;
		else if(i >= 256)
			ed->size = 128;
		else if(oed->gfxtmp == 0xff)
			ed->size = 256, i = 0;
		else
			ed->size = 64;
		
		ed->blknum = i;
		ed->buf = malloc(ed->size << 6);
		Blkedit8load(ed);

		ed->scrollh = 0;
		ed->scrollv = 0;
		ed->bmih = zbmih;
		ed->bmih.biWidth = 128;
		ed->bmih.biHeight = ed->size >> 1;
		memcpy(ed->pal, oed->pal, 1024);
		
		ed->blkwnd = hc;
		hc = GetDlgItem(win, IDC_CUSTOM2);
		GetClientRect(hc, &rc);
		
		ed->pwidth = rc.right;
		ed->pheight = rc.bottom;
		ed->psel = i = ((short*) lparam)[3] << 4;
		
		if(ed->blknum != 260 && ed->blknum >= 256)
			i >>= 2;

		for(j = (ed->size << 6) - 1; j >= 0; j--) 
		{
			if(ed->buf[j]) 
				ed->buf[j] |= i;
		}
	
		SetWindowLong(hc,GWL_USERDATA,(int)ed);
		
		break;

	case WM_DESTROY:
	
		ed = (BLKEDIT8*) GetWindowLong(win, DWL_USER);

		// Not dummied by MONapkins!!! (I think and hope)
		// dummied:
		//		for(i=0;i<256;i++) DeleteObject(ed->brush[i]);
	
		free(ed->buf);
		free(ed);
	
		break;

	case WM_COMMAND:
	
		ed = (BLKEDIT8*) GetWindowLong(win, DWL_USER);
	
		switch(wparam)
		{
		case IDC_BUTTON1:
		
			if(ed->oed->gfxtmp == 0xff) 
			{
				hgl = GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,17448);
				b = GlobalLock(hgl);
				
				ed->bmih.biSizeImage = ed->bmih.biWidth*ed->bmih.biHeight;
				
				if( (hc = ed->oed->hpal) != 0) 
				{
					memcpy(b, &(ed->bmih), 40);
					
					b += 40;

					for(j = 0; j < 256; j += 8)
					{
						GetPaletteEntries(hc, ((short*)(ed->pal))[j], 8, (void*)(o));
						
						for(i = 0; i < 8; i++)
						{
							((int*)(b))[i] = ((o[i] & 0xFF)<<16) | ((o[i] & 0xFF0000) >> 16) | (o[i] & 0xFF00);
						}
						
						b += 32;
					}
				} 
				else 
				{	
					memcpy(b, &(ed->bmih), 1064);
					b += 1064;
				}
				
				memcpy(b,ed->buf,16384);
			} 
			else 
			{
				bmih.biSize=40;
				bmih.biWidth=128;
				bmih.biHeight=ed->bmih.biHeight;
				bmih.biPlanes=1;
				bmih.biBitCount=4;
				bmih.biCompression=BI_RGB;
				bmih.biSizeImage=bmih.biWidth*bmih.biHeight>>1;
				bmih.biClrUsed=ed->blknum==260?16:8;
				bmih.biClrImportant=0;
				hgl=GlobalAlloc(GMEM_MOVEABLE|GMEM_DDESHARE,40+(bmih.biClrUsed<<2)+(ed->size<<5));
				b=GlobalLock(hgl);
				*(BITMAPINFOHEADER*)b=bmih;
				j=ed->psel;
				if(ed->blknum>=256) j&=0xfc; else j&=0xf8;
				k=ed->size<<5;
				b2=ed->buf;
				b+=40;
				l=bmih.biClrUsed;
				
				if( (hc = ed->oed->hpal) != 0 )
				{
					GetPaletteEntries(hc, 0, 1, (void*) o);
					GetPaletteEntries(hc, ((short*) (ed->pal))[1 + j], l - 1, (void*) (o + 1));
				
					for(i = 0; i < l; i++)
					{
						((int*) (b))[i] = ((o[i] & 0xff) << 16) | ( (o[i] & 0xff0000) >> 16) | (o[i] & 0xff00);
					}
				} 
				else 
				{
					*(int*)(b) = *(int*)ed->pal;
		
					for(i = 1; i < l; i++)
						((int*) (b))[i] = ((int*) ed->pal)[i + j];
				}

				b += bmih.biClrUsed << 2;
				j = 0;
				l--;
				
				for(i = 0; i < k; i++)
				{
					*(b++) = ((b2[j] & l) << 4) | (b2[j + 1] & l);
					j += 2;
				}
			}
	
			GlobalUnlock(hgl);
			OpenClipboard(0);
			EmptyClipboard();
			SetClipboardData(CF_DIB, hgl);
			CloseClipboard();
			
			break;
		case IDC_BUTTON2:
			OpenClipboard(0);
			hgl=GetClipboardData(CF_DIB);
			if(!hgl) {
				MessageBox(framewnd,"Nothing is on the clipboard.","Bad error happened",MB_OK);
				CloseClipboard();
				break;
			}
			b=GlobalLock(hgl);
			bmih=*(BITMAPINFOHEADER*)b;
			if(bmih.biSize!=40 || bmih.biWidth!=128 ||
				(bmih.biHeight!=ed->bmih.biHeight&&bmih.biHeight!=-ed->bmih.biHeight) || bmih.biPlanes!=1
				|| (bmih.biBitCount!=4 && bmih.biBitCount!=8) ||
				bmih.biCompression!=BI_RGB) {
				GlobalUnlock(hgl);
				CloseClipboard();
				MessageBox(framewnd,"The image does not have the correct dimensions or is not a 16 color image.","Bad error happened",MB_OK);
				break;
			}
			b+=40+(((bmih.biClrUsed?bmih.biClrUsed:(1<<bmih.biBitCount)))<<2);
			if(ed->blknum==260) k=15; else k=7;
			if(bmih.biBitCount==8)
				memcpy(ed->buf,b,ed->size<<6);
			else {
				j=ed->size<<5;
				b2=ed->buf;
				for(i=0;i<j;i++) {
					*(b2++)=((*b)>>4);
					*(b2++)=(*(b++));
				}
			}
			if(ed->oed->gfxtmp!=0xff) {
				if(ed->blknum==260) j=ed->psel&0xf0; else j=ed->psel&0xf8;
				for(i=(ed->size<<6)-1;i>=0;i--) {
					ed->buf[i]&=k;
					if(ed->buf[i]) ed->buf[i]|=j;
				}
			}
			if(bmih.biHeight<0) {
				j=(ed->size<<5);
				k=(ed->size<<6)-128;
				for(i=0;i<j;i++) {
					l=ed->buf[i];
					ed->buf[i]=ed->buf[i^k];
					ed->buf[i^k]=l;
				}
			}
			GlobalUnlock(hgl);
			CloseClipboard();
			InvalidateRect(ed->blkwnd,0,0);
			break;
		case IDOK:
			oed=ed->oed;
			doc=oed->ew.doc;
			blk=doc->blks;
			j=ed->size<<6;
			b2=malloc(j);
			for(i=0;i<j;i++)
				b2[(i&7)+((i&0x78)<<3)+((i&0x380)>>4)+(i&0xfc00)]=ed->buf[j-128-(i&0xff80)+(i&127)];
			i=2;
			if(ed->blknum==260) {memcpy(blk[k=225].buf,b2,0xe000);goto endsave3;}
			if(ed->blknum==259) {memcpy(blk[k=224].buf,b2,0x4000);goto endsave3;}
			if(ed->blknum>=256) {memcpy(blk[k=bg3blkofs[ed->blknum-256]].buf,b2,0x2000);goto endsave3;}
			if(ed->blknum>=32) {memcpy(blk[k=ed->blknum-32].buf,b2,0x1000);goto endsave3;}
			if(oed->gfxtmp==0xff) {
				memcpy(blk[223].buf,b2,16384);
				blk[223].modf=1;
				goto endsave2;
			} else {
				j=ed->size<<6;
				for(i=0;i<j;i++) {
					b2[i]&=7;
					if(b2[i]) b2[i]|=8;
				}
				j=ed->blknum;
				m[0]=0;
				m[1]=0;
				if(oed->gfxtmp>=0x20) { if(ed->blknum==7) {
					memcpy(blk[oed->blocksets[7]].buf+0x800,b2+0x800,0x800);
					if(oed->anim==2) {
						memcpy(blk[oed->blocksets[9]].buf,b2,0x800);
						m[1]=1;
					} else if(oed->anim==1) {
						memcpy(blk[oed->blocksets[8]].buf+0x800,b2,0x800);
						m[0]=1;
					} else {
						memcpy(blk[oed->blocksets[8]].buf,b2,0x800);
						m[0]=1;
					}
					goto endsave;
				} } else {
					if(ed->blknum==6) {
						memcpy(blk[oed->blocksets[6]].buf,b2,0xc00);
						memcpy(blk[oed->blocksets[8]].buf+(oed->anim<<10),b2+0xc00,0x400);
						m[0]=1;
						goto endsave;
					} 
					else if(ed->blknum==7) {
						memcpy(blk[oed->blocksets[7]].buf+0x400,b2+0x400,0xc00);
						memcpy(blk[oed->blocksets[9]].buf+(oed->anim<<10),b2,0x400);
						m[1] = 1;
					
						goto endsave;
					}
				}
			}
	
			memcpy(blk[(j >= 15) ? (0x6a + j) : oed->blocksets[j]].buf, b2, 0x1000);
endsave:
			for(i = 0; i < 3; i++)
			{
				if(i == 2) 
					k = (j >= 15) ? (0x6a + j) : oed->blocksets[j];
				else if(m[i]) 
					k = oed->blocksets[i + 8];
				else 
					continue;
endsave3:
				b = blk[k].buf;
		
				if(k == 225) 
					l = 0xe000; 
				else if(k == 224) 
					l = 0x4000; 
				else if(k >= 220) 
					l = 0x2000;
				else l = 0x1000;
				
				for(n = 0; n < l; n++)
					b[n + l] = masktab[b[n]];
				
				for(n = 0; n < l; n++)
					b[n+l+l]=b[n^7];
				
				b += l << 1;
				
				for(n = 0; n < l; n++)
					b[n + l] = masktab[b[n]];
				
				blk[k].modf=1;
			}
		
			for(i = 0; i < 160; i++)
				if( (hc = doc->overworld[i].win) != 0)
				{
					hc2 = GetDlgItem(hc, 2000);
					hc = GetDlgItem(hc2, 3000);
					InvalidateRect(hc,0,0);
					
					hc = GetDlgItem(hc2,3001);
					InvalidateRect(hc, 0, 0);
				}
			
			for(i = 0; i < 0xa8; i++)
				if( (hc = doc->ents[i]) != 0 )
				{
					hc2 = GetDlgItem