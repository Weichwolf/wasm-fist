  
  scratch/loader.bin:     file format binary
  
  
  Disassembly of section .data:
  
  00000000 <.data>:
         0:	72 f0                	jb     0xfffffff2
         2:	3b c8                	cmp    %ax,%cx
         4:	75 ec                	jne    0xfffffff2
         6:	36 66 89 36 84 01    	mov    %esi,%ss:0x184
         c:	57                   	push   %di
         d:	36 8b 3e 00 00       	mov    %ss:0x0,%di
        12:	8b 0e 06 00          	mov    0x6,%cx
        16:	8b 36 18 00          	mov    0x18,%si
        1a:	66 c1 cb 10          	ror    $0x10,%ebx
        1e:	66 0f b7 c3          	movzwl %bx,%eax
        22:	8b d0                	mov    %ax,%dx
        24:	ad                   	lods   %ds:(%si),%ax
        25:	8b 1c                	mov    (%si),%bx
        27:	3b df                	cmp    %di,%bx
        29:	0f 8d 08 00          	jge    0x35
        2d:	03 da                	add    %dx,%bx
        2f:	8e c3                	mov    %bx,%es
        31:	26 67 01 10          	add    %dx,%es:(%eax)
        35:	83 c6 02             	add    $0x2,%si
        38:	e2 ea                	loop   0x24
        3a:	5f                   	pop    %di
        3b:	36 8c 1e 7e 01       	mov    %ds,%ss:0x17e
        40:	16                   	push   %ss
        41:	1f                   	pop    %ds
        42:	66 8f 06 88 01       	popl   0x188
        47:	66 8f 06 8c 01       	popl   0x18c
        4c:	66 8f 06 90 01       	popl   0x190
        51:	66 8f 06 94 01       	popl   0x194
        56:	66 8f 06 98 01       	popl   0x198
        5b:	66 89 26 14 01       	mov    %esp,0x114
        60:	66 89 26 78 01       	mov    %esp,0x178
        65:	66 89 1e 80 01       	mov    %ebx,0x180
        6a:	8c 2e 7c 01          	mov    %gs,0x17c
        6e:	66 55                	push   %ebp
        70:	8f 06 c4 04          	pop    0x4c4
        74:	5b                   	pop    %bx
        75:	88 1e fc 04          	mov    %bl,0x4fc
        79:	80 fb c0             	cmp    $0xc0,%bl
        7c:	0f 84 80 36          	je     0x3700
        80:	66 89 3e fe 04       	mov    %edi,0x4fe
        85:	68 00 32             	push   $0x3200
        88:	9d                   	popf
        89:	66 33 c0             	xor    %eax,%eax
        8c:	8c d0                	mov    %ss,%ax
        8e:	66 c1 e0 04          	shl    $0x4,%eax
        92:	66 01 06 22 00       	add    %eax,0x22
        97:	66 01 06 2a 00       	add    %eax,0x2a
        9c:	66 01 06 62 00       	add    %eax,0x62
        a1:	66 01 06 d2 00       	add    %eax,0xd2
        a6:	66 33 c0             	xor    %eax,%eax
        a9:	8c e8                	mov    %gs,%ax
        ab:	66 c1 e0 04          	shl    $0x4,%eax
        af:	66 01 06 52 00       	add    %eax,0x52
        b4:	66 33 c0             	xor    %eax,%eax
        b7:	8c e0                	mov    %fs,%ax
        b9:	66 c1 e0 04          	shl    $0x4,%eax
        bd:	66 01 06 5a 00       	add    %eax,0x5a
        c2:	66 33 c0             	xor    %eax,%eax
        c5:	8c c8                	mov    %cs,%ax
        c7:	66 c1 e0 04          	shl    $0x4,%eax
        cb:	66 01 06 1a 00       	add    %eax,0x1a
        d0:	66 a1 cc 04          	mov    0x4cc,%eax
        d4:	66 25 ff ff ff 00    	and    $0xffffff,%eax
        da:	66 09 06 4a 00       	or     %eax,0x4a
        df:	66 09 06 42 00       	or     %eax,0x42
        e4:	66 09 06 3a 00       	or     %eax,0x3a
        e9:	a0 cf 04             	mov    0x4cf,%al
        ec:	a2 4f 00             	mov    %al,0x4f
        ef:	a2 47 00             	mov    %al,0x47
        f2:	a2 3f 00             	mov    %al,0x3f
        f5:	e8 d1 30             	call   0x31c9
        f8:	e8 7e 28             	call   0x2979
        fb:	80 3e fc 04 c4       	cmpb   $0xc4,0x4fc
       100:	0f 84 59 35          	je     0x365d
       104:	e8 52 1d             	call   0x1e59
       107:	e8 02 31             	call   0x320c
       10a:	0f 20 c0             	mov    %cr0,%eax
       10d:	66 a3 08 01          	mov    %eax,0x108
       111:	66 0d 01 00 00 80    	or     $0x80000001,%eax
       117:	24 fb                	and    $0xfb,%al
       119:	66 a3 0c 01          	mov    %eax,0x10c
       11d:	0f 01 0e 02 01       	sidtw  0x102
       122:	b9 10 01             	mov    $0x110,%cx
       125:	66 33 c0             	xor    %eax,%eax
       128:	b0 07                	mov    $0x7,%al
       12a:	66 ab                	stos   %eax,%es:(%di)
       12c:	66 05 00 10 00 00    	add    $0x1000,%eax
       132:	e2 f6                	loop   0x12a
       134:	e8 da 28             	call   0x2a11
       137:	2e ff 16 16 14       	call   *%cs:0x1416
       13c:	16                   	push   %ss
       13d:	1f                   	pop    %ds
       13e:	e8 34 33             	call   0x3475
       141:	66 cb                	lretl
       143:	66 0f b7 e4          	movzwl %sp,%esp
       147:	1e                   	push   %ds
       148:	06                   	push   %es
       149:	0f a0                	push   %fs
       14b:	0f a8                	push   %gs
       14d:	9c                   	pushf
       14e:	54                   	push   %sp
       14f:	16                   	push   %ss
       150:	66 53                	push   %ebx
       152:	66 50                	push   %eax
       154:	bb 00 00             	mov    $0x0,%bx
       157:	8e c3                	mov    %bx,%es
       159:	26 81 2e 14 01 80 01 	subw   $0x180,%es:0x114
       160:	16                   	push   %ss
       161:	1f                   	pop    %ds
       162:	8b c4                	mov    %sp,%ax
       164:	57                   	push   %di
       165:	56                   	push   %si
       166:	51                   	push   %cx
       167:	8b f0                	mov    %ax,%si
       169:	26 8b 3e 14 01       	mov    %es:0x114,%di
       16e:	b9 09 00             	mov    $0x9,%cx
       171:	fc                   	cld
       172:	f3 66 a5             	rep movsl %ds:(%si),%es:(%di)
       175:	8e db                	mov    %bx,%ds
       177:	66 8b 45 f6          	mov    -0xa(%di),%eax
       17b:	59                   	pop    %cx
       17c:	5e                   	pop    %si
       17d:	5f                   	pop    %di
       17e:	83 c4 24             	add    $0x24,%sp
       181:	2e 66 ff 36 16 10    	pushl  %cs:0x1016
       187:	66 68 04 bf 00 00    	pushl  $0xbf04
       18d:	53                   	push   %bx
       18e:	66 8b 1e 14 01       	mov    0x114,%ebx
       193:	17                   	pop    %ss
       194:	66 87 dc             	xchg   %ebx,%esp
       197:	2e 66 ff 36 14 10    	pushl  %cs:0x1014
       19d:	67 66 0f b7 44 24 0c 	movzwl 0xc(%esp),%eax
       1a4:	66 c1 e0 04          	shl    $0x4,%eax
       1a8:	67 66 8d 04 18       	lea    (%eax,%ebx,1),%eax
       1ad:	66 50                	push   %eax
       1af:	8b 1e 14 01          	mov    0x114,%bx
       1b3:	2e 66 ff 36 16 10    	pushl  %cs:0x1016
       1b9:	36 66 ff 77 1a       	pushl  %ss:0x1a(%bx)
       1be:	ff 77 0c             	push   0xc(%bx)
       1c1:	66 ff 77 04          	pushl  0x4(%bx)
       1c5:	66 ff 37             	pushl  (%bx)
       1c8:	8b 47 22             	mov    0x22(%bx),%ax
       1cb:	0b c0                	or     %ax,%ax
       1cd:	0f 84 10 00          	je     0x1e1
       1d1:	8e c0                	mov    %ax,%es
       1d3:	8b 5f 20             	mov    0x20(%bx),%bx
       1d6:	26 66 ff 77 04       	pushl  %es:0x4(%bx)
       1db:	26 66 ff 37          	pushl  %es:(%bx)
       1df:	eb 0d                	jmp    0x1ee
       1e1:	66 6a 00             	pushl  $0x0
       1e4:	2e ff 36 18 10       	push   %cs:0x1018
       1e9:	2e ff 36 18 10       	push   %cs:0x1018
       1ee:	33 c0                	xor    %ax,%ax
       1f0:	2e ff 16 16 14       	call   *%cs:0x1416
       1f5:	1f                   	pop    %ds
       1f6:	07                   	pop    %es
       1f7:	0f a1                	pop    %fs
       1f9:	0f a9                	pop    %gs
       1fb:	66 58                	pop    %eax
       1fd:	66 5b                	pop    %ebx
       1ff:	36 80 3e fc 04 c0    	cmpb   $0xc0,%ss:0x4fc
       205:	0f 84 8b 00          	je     0x294
       209:	9d                   	popf
       20a:	66 cb                	lretl
       20c:	2e 8e 16 12 10       	mov    %cs:0x1012,%ss
       211:	36 66 8b 26 14 01    	mov    %ss:0x114,%esp
       217:	0f a8                	push   %gs
       219:	0f a0                	push   %fs
       21b:	06                   	push   %es
       21c:	1e                   	push   %ds
       21d:	9c                   	pushf
       21e:	66 50                	push   %eax
       220:	66 56                	push   %esi
       222:	b8 01 00             	mov    $0x1,%ax
       225:	2e ff 16 14 14       	call   *%cs:0x1414
       22a:	8b 36 14 01          	mov    0x114,%si
       22e:	8e 54 08             	mov    0x8(%si),%ss
       231:	8b 64 0a             	mov    0xa(%si),%sp
       234:	83 c4 18             	add    $0x18,%sp
       237:	66 6a 00             	pushl  $0x0
       23a:	66 6a 00             	pushl  $0x0
       23d:	6a 00                	push   $0x0
       23f:	66 ff 74 16          	pushl  0x16(%si)
       243:	66 ff 74 12          	pushl  0x12(%si)
       247:	66 ff 74 0e          	pushl  0xe(%si)
       24b:	ff 74 f6             	push   -0xa(%si)
       24e:	66 ff 74 f2          	pushl  -0xe(%si)
       252:	66 ff 74 ee          	pushl  -0x12(%si)
       256:	57                   	push   %di
       257:	51                   	push   %cx
       258:	8b 44 22             	mov    0x22(%si),%ax
       25b:	0b c0                	or     %ax,%ax
       25d:	0f 84 0f 00          	je     0x270
       261:	8e c0                	mov    %ax,%es
       263:	8b 7c 20             	mov    0x20(%si),%di
       266:	b9 02 00             	mov    $0x2,%cx
       269:	83 ee 08             	sub    $0x8,%si
       26c:	fc                   	cld
       26d:	f3 66 a5             	rep movsl %ds:(%si),%es:(%di)
       270:	81 06 14 01 80 01    	addw   $0x180,0x114
       276:	59                   	pop    %cx
       277:	5f                   	pop    %di
       278:	9c                   	pushf
       279:	58                   	pop    %ax
       27a:	67 80 64 24 09 fc    	andb   $0xfc,0x9(%esp)
       280:	80 e4 02             	and    $0x2,%ah
       283:	67 08 64 24 09       	or     %ah,0x9(%esp)
       288:	66 5e                	pop    %esi
       28a:	66 58                	pop    %eax
       28c:	9d                   	popf
       28d:	0f a9                	pop    %gs
       28f:	0f a1                	pop    %fs
       291:	07                   	pop    %es
       292:	1f                   	pop    %ds
       293:	cb                   	lret
       294:	56                   	push   %si
       295:	8b f4                	mov    %sp,%si
       297:	16                   	push   %ss
       298:	0f a9                	pop    %gs
       29a:	2e 8e 16 14 10       	mov    %cs:0x1014,%ss
       29f:	65 66 8b 64 0c       	mov    %gs:0xc(%si),%esp
       2a4:	65 66 ff 74 08       	pushl  %gs:0x8(%si)
       2a9:	65 66 ff 74 04       	pushl  %gs:0x4(%si)
       2ae:	65 66 ff 34          	pushl  %gs:(%si)
       2b2:	6a 00                	push   $0x0
       2b4:	0f a9                	pop    %gs
       2b6:	5e                   	pop    %si
       2b7:	67 f7 04 24 00 02    	testw  $0x200,(%esp)
       2bd:	fa                   	cli
       2be:	74 01                	je     0x2c1
       2c0:	fb                   	sti
       2c1:	9d                   	popf
       2c2:	66 cb                	lretl
       2c4:	66 0f b7 e4          	movzwl %sp,%esp
       2c8:	1e                   	push   %ds
       2c9:	06                   	push   %es
       2ca:	0f a0                	push   %fs
       2cc:	0f a8                	push   %gs
       2ce:	66 50                	push   %eax
       2d0:	9c                   	pushf
       2d1:	55                   	push   %bp
       2d2:	2e 8e 06 12 10       	mov    %cs:0x1012,%es
       2d7:	26 8b 2e 14 01       	mov    %es:0x114,%bp
       2dc:	89 66 00             	mov    %sp,0x0(%bp)
       2df:	8b ec                	mov    %sp,%bp
       2e1:	8a 66 13             	mov    0x13(%bp),%ah
       2e4:	80 e4 3f             	and    $0x3f,%ah
       2e7:	0f 84 25 00          	je     0x310
       2eb:	66 0f b6 c4          	movzbl %ah,%eax
       2ef:	d1 e0                	shl    $1,%ax
       2f1:	2b e0                	sub    %ax,%sp
       2f3:	66 57                	push   %edi
       2f5:	66 56                	push   %esi
       2f7:	66 51                	push   %ecx
       2f9:	66 c5 76 20          	lds    0x20(%bp),%esi
       2fd:	67 66 8d 7c 24 0c    	lea    0xc(%esp),%edi
       303:	66 8b c8             	mov    %eax,%ecx
       306:	fc                   	cld
       307:	67 f3 a4             	rep movsb %ds:(%esi),%es:(%edi)
       30a:	66 59                	pop    %ecx
       30c:	66 5e                	pop    %esi
       30e:	66 5f                	pop    %edi
       310:	33 c0                	xor    %ax,%ax
       312:	2e ff 16 14 14       	call   *%cs:0x1414
       317:	8e 5e 14             	mov    0x14(%bp),%ds
       31a:	8e 46 16             	mov    0x16(%bp),%es
       31d:	8e 66 18             	mov    0x18(%bp),%fs
       320:	8e 6e 1a             	mov    0x1a(%bp),%gs
       323:	9c                   	pushf
       324:	58                   	pop    %ax
       325:	80 66 03 bc          	andb   $0xbc,0x3(%bp)
       329:	80 e4 02             	and    $0x2,%ah
       32c:	08 66 03             	or     %ah,0x3(%bp)
       32f:	f6 46 13 80          	testb  $0x80,0x13(%bp)
       333:	74 63                	je     0x398
       335:	0e                   	push   %cs
       336:	68 82 13             	push   $0x1382
       339:	66 ff 76 1c          	pushl  0x1c(%bp)
       33d:	66 8b 46 04          	mov    0x4(%bp),%eax
       341:	ff 76 02             	push   0x2(%bp)
       344:	9d                   	popf
       345:	8b 6e 00             	mov    0x0(%bp),%bp
       348:	cb                   	lret
       349:	9d                   	popf
       34a:	cd 00                	int    $0x0
       34c:	55                   	push   %bp
       34d:	36 8b 2e 14 01       	mov    %ss:0x114,%bp
       352:	8b 6e 00             	mov    0x0(%bp),%bp
       355:	66 89 46 04          	mov    %eax,0x4(%bp)
       359:	9c                   	pushf
       35a:	8f 46 02             	pop    0x2(%bp)
       35d:	8f 46 00             	pop    0x0(%bp)
       360:	f6 46 13 80          	testb  $0x80,0x13(%bp)
       364:	75 0b                	jne    0x371
       366:	2e a0 81 13          	mov    %cs:0x1381,%al
       36a:	86 46 12             	xchg   %al,0x12(%bp)
       36d:	2e a2 81 13          	mov    %al,%cs:0x1381
       371:	8c 5e 14             	mov    %ds,0x14(%bp)
       374:	8c 46 16             	mov    %es,0x16(%bp)
       377:	8c 66 18             	mov    %fs,0x18(%bp)
       37a:	8c 6e 1a             	mov    %gs,0x1a(%bp)
       37d:	b8 01 00             	mov    $0x1,%ax
       380:	2e ff 16 16 14       	call   *%cs:0x1416
       385:	8b e5                	mov    %bp,%sp
       387:	5d                   	pop    %bp
       388:	67 81 24 24 ff bf    	andw   $0xbfff,(%esp)
       38e:	9d                   	popf
       38f:	66 58                	pop    %eax
       391:	0f a9                	pop    %gs
       393:	0f a1                	pop    %fs
       395:	07                   	pop    %es
       396:	1f                   	pop    %ds
       397:	c3                   	ret
       398:	2e 66 0f b6 06 81 13 	movzbl %cs:0x1381,%eax
       39f:	86 46 12             	xchg   %al,0x12(%bp)
       3a2:	2e a2 81 13          	mov    %al,%cs:0x1381
       3a6:	c1 e0 02             	shl    $0x2,%ax
       3a9:	ff 76 02             	push   0x2(%bp)
       3ac:	1e                   	push   %ds
       3ad:	36 8e 1e 04 02       	mov    %ss:0x204,%ds
       3b2:	67 f6 40 03 01       	testb  $0x1,0x3(%eax)
       3b7:	0f 85 0a 00          	jne    0x3c5
       3bb:	66 8b 46 04          	mov    0x4(%bp),%eax
       3bf:	8b 6e 00             	mov    0x0(%bp),%bp
       3c2:	1f                   	pop    %ds
       3c3:	eb 84                	jmp    0x349
       3c5:	36 8e 1e 1c 02       	mov    %ss:0x21c,%ds
       3ca:	67 66 8b 00          	mov    (%eax),%eax
       3ce:	1f                   	pop    %ds
       3cf:	0e                   	push   %cs
       3d0:	68 82 13             	push   $0x1382
       3d3:	66 50                	push   %eax
       3d5:	66 8b 46 04          	mov    0x4(%bp),%eax
       3d9:	8b 6e 00             	mov    0x0(%bp),%bp
       3dc:	fa                   	cli
       3dd:	cb                   	lret
       3de:	18 14                	sbb    %dl,(%si)
       3e0:	0f 15 9c 2e a1       	unpckhps -0x5ed2(%si),%xmm3
       3e5:	12 10                	adc    (%bx,%si),%dl
       3e7:	8e d8                	mov    %ax,%ds
       3e9:	8e c0                	mov    %ax,%es
       3eb:	8e e0                	mov    %ax,%fs
       3ed:	8e e8                	mov    %ax,%gs
       3ef:	fa                   	cli
       3f0:	0f 01 1e 02 01       	lidtw  0x102
       3f5:	66 a1 08 01          	mov    0x108,%eax
       3f9:	0f 22 c0             	mov    %eax,%cr0
       3fc:	ea 37 14 6f 00       	ljmp   $0x6f,$0x1437
       401:	0f 22 dc             	mov    %esp,%cr3
       404:	b8 00 00             	mov    $0x0,%ax
       407:	8e d0                	mov    %ax,%ss
       409:	8e d8                	mov    %ax,%ds
       40b:	2e 80 3e 0e 15 00    	cmpb   $0x0,%cs:0x150e
       411:	75 02                	jne    0x415
       413:	9d                   	popf
       414:	c3                   	ret
       415:	52                   	push   %dx
       416:	51                   	push   %cx
       417:	b1 00                	mov    $0x0,%cl
       419:	e8 10 2c             	call   0x302c
       41c:	59                   	pop    %cx
       41d:	5a                   	pop    %dx
       41e:	2e c6 06 0e 15 00    	movb   $0x0,%cs:0x150e
       424:	9d                   	popf
       425:	c3                   	ret
       426:	9c                   	pushf
       427:	66 0f b7 c4          	movzwl %sp,%eax
       42b:	66 68 00 00 00 00    	pushl  $0x0
       431:	66 68 00 00 00 00    	pushl  $0x0
       437:	66 68 00 00 00 00    	pushl  $0x0
       43d:	66 50                	push   %eax
       43f:	66 50                	push   %eax
       441:	66 68 6f 00 00 00    	pushl  $0x6f
       447:	66 68 92 14 00 00    	pushl  $0x1492
       44d:	b8 0c de             	mov    $0xde0c,%ax
       450:	2e 8e 1e 14 10       	mov    %cs:0x1014,%ds
       455:	fa                   	cli
       456:	66 2e ff 1e 22 10    	lcalll *%cs:0x1022
       45c:	9d                   	popf
       45d:	c3                   	ret
       45e:	16                   	push   %ss
       45f:	16                   	push   %ss
       460:	07                   	pop    %es
       461:	1f                   	pop    %ds
       462:	66 60                	pushal
       464:	9c                   	pushf
       465:	50                   	push   %ax
       466:	50                   	push   %ax
       467:	b8 00 09             	mov    $0x900,%ax
       46a:	cd 31                	int    $0x31
       46c:	8b e8                	mov    %ax,%bp
       46e:	58                   	pop    %ax
       46f:	0a c0                	or     %al,%al
       471:	0f 85 13 00          	jne    0x488
       475:	66 0f b7 3e 28 00    	movzwl 0x28,%edi
       47b:	2b 3e 20 00          	sub    0x20,%di
       47f:	89 3e 28 00          	mov    %di,0x28
       483:	66 ff 1e 10 00       	lcalll *0x10
       488:	b8 00 00             	mov    $0x0,%ax
       48b:	8b c8                	mov    %ax,%cx
       48d:	8b d0                	mov    %ax,%dx
       48f:	66 8b dc             	mov    %esp,%ebx
       492:	be 6f 00             	mov    $0x6f,%si
       495:	bf d3 14             	mov    $0x14d3,%di
       498:	66 ff 2e 30 00       	ljmpl  *0x30
       49d:	58                   	pop    %ax
       49e:	0a c0                	or     %al,%al
       4a0:	0f 84 12 00          	je     0x4b6
       4a4:	66 0f b7 3e 28 00    	movzwl 0x28,%edi
       4aa:	ff 1e 18 00          	lcall  *0x18
       4ae:	03 3e 20 00          	add    0x20,%di
       4b2:	89 3e 28 00          	mov    %di,0x28
       4b6:	58                   	pop    %ax
       4b7:	8b dd                	mov    %bp,%bx
       4b9:	80 e4 fc             	and    $0xfc,%ah
       4bc:	d0 c3                	rol    $1,%bl
       4be:	0a e3                	or     %bl,%ah
       4c0:	50                   	push   %ax
       4c1:	9d                   	popf
       4c2:	66 61                	popal
       4c4:	c3                   	ret
       4c5:	66 cb                	lretl
       4c7:	52                   	push   %dx
       4c8:	51                   	push   %cx
       4c9:	b1 01                	mov    $0x1,%cl
       4cb:	e8 5e 2b             	call   0x302c
       4ce:	2e c6 06 0e 15 01    	movb   $0x1,%cs:0x150e
       4d4:	59                   	pop    %cx
       4d5:	5a                   	pop    %dx
       4d6:	eb 29                	jmp    0x501
       4d8:	00 9c 33 c0          	add    %bl,-0x3fcd(%si)
       4dc:	8e c0                	mov    %ax,%es
       4de:	8e e0                	mov    %ax,%fs
       4e0:	8e e8                	mov    %ax,%gs
       4e2:	50                   	push   %ax
       4e3:	48                   	dec    %ax
       4e4:	8e d8                	mov    %ax,%ds
       4e6:	fa                   	cli
       4e7:	26 8a 26 00 00       	mov    %es:0x0,%ah
       4ec:	fe cc                	dec    %ah
       4ee:	8a c4                	mov    %ah,%al
       4f0:	86 06 10 00          	xchg   %al,0x10
       4f4:	26 2a 26 00 00       	sub    %es:0x0,%ah
       4f9:	86 06 10 00          	xchg   %al,0x10
       4fd:	1f                   	pop    %ds
       4fe:	9e                   	sahf
       4ff:	73 c6                	jae    0x4c7
       501:	36 0f 01 16 d0 00    	lgdtw  %ss:0xd0
       507:	36 0f 01 1e fc 00    	lidtw  %ss:0xfc
       50d:	36 66 a1 2c 01       	mov    %ss:0x12c,%eax
       512:	0f 22 d8             	mov    %eax,%cr3
       515:	36 66 a1 0c 01       	mov    %ss:0x10c,%eax
       51a:	0f 22 c0             	mov    %eax,%cr0
       51d:	ea 58 15 08 00       	ljmp   $0x8,$0x1558
       522:	2e 8e 16 12 10       	mov    %cs:0x1012,%ss
       527:	36 80 26 65 00 fd    	andb   $0xfd,%ss:0x65
       52d:	2e 0f 00 1e 20 10    	ltr    %cs:0x1020
       533:	58                   	pop    %ax
       534:	80 e4 bf             	and    $0xbf,%ah
       537:	80 cc 30             	or     $0x30,%ah
       53a:	50                   	push   %ax
       53b:	9d                   	popf
       53c:	c3                   	ret
       53d:	9c                   	pushf
       53e:	66 51                	push   %ecx
       540:	66 56                	push   %esi
       542:	b8 0c de             	mov    $0xde0c,%ax
       545:	36 66 8b 36 fa 05    	mov    %ss:0x5fa,%esi
       54b:	66 8b cc             	mov    %esp,%ecx
       54e:	fa                   	cli
       54f:	36 c7 06 f4 05 8e 15 	movw   $0x158e,%ss:0x5f4
       556:	cd 67                	int    $0x67
       558:	2e 8e 16 12 10       	mov    %cs:0x1012,%ss
       55d:	33 c0                	xor    %ax,%ax
       55f:	8e d8                	mov    %ax,%ds
       561:	8e c0                	mov    %ax,%es
       563:	8e e0                	mov    %ax,%fs
       565:	8e e8                	mov    %ax,%gs
       567:	66 8b e1             	mov    %ecx,%esp
       56a:	66 5e                	pop    %esi
       56c:	66 59                	pop    %ecx
       56e:	67 81 24 24 ff bf    	andw   $0xbfff,(%esp)
       574:	9d                   	popf
       575:	c3                   	ret
       576:	16                   	push   %ss
       577:	16                   	push   %ss
       578:	07                   	pop    %es
       579:	1f                   	pop    %ds
       57a:	66 60                	pushal
       57c:	9c                   	pushf
       57d:	5d                   	pop    %bp
       57e:	50                   	push   %ax
       57f:	fa                   	cli
       580:	0a c0                	or     %al,%al
       582:	0f 85 12 00          	jne    0x598
       586:	66 0f b7 3e 28 00    	movzwl 0x28,%edi
       58c:	2b 3e 20 00          	sub    0x20,%di
       590:	89 3e 28 00          	mov    %di,0x28
       594:	ff 1e 18 00          	lcall  *0x18
       598:	2e a1 12 10          	mov    %cs:0x1012,%ax
       59c:	8b c8                	mov    %ax,%cx
       59e:	8b d0                	mov    %ax,%dx
       5a0:	66 8b dc             	mov    %esp,%ebx
       5a3:	2e 8b 36 10 10       	mov    %cs:0x1010,%si
       5a8:	66 bf e8 15 00 00    	mov    $0x15e8,%edi
       5ae:	ff 2e 38 00          	ljmp   *0x38
       5b2:	58                   	pop    %ax
       5b3:	0a c0                	or     %al,%al
       5b5:	0f 84 13 00          	je     0x5cc
       5b9:	66 0f b7 3e 28 00    	movzwl 0x28,%edi
       5bf:	66 ff 1e 10 00       	lcalll *0x10
       5c4:	03 3e 20 00          	add    0x20,%di
       5c8:	89 3e 28 00          	mov    %di,0x28
       5cc:	33 c0                	xor    %ax,%ax
       5ce:	8e d8                	mov    %ax,%ds
       5d0:	8e c0                	mov    %ax,%es
       5d2:	f7 c5 00 02          	test   $0x200,%bp
       5d6:	66 61                	popal
       5d8:	74 01                	je     0x5db
       5da:	fb                   	sti
       5db:	c3                   	ret
       5dc:	14 16                	adc    $0x16,%al
       5de:	66 0f b7 e4          	movzwl %sp,%esp
       5e2:	1e                   	push   %ds
       5e3:	06                   	push   %es
       5e4:	0f a0                	push   %fs
       5e6:	0f a8                	push   %gs
       5e8:	66 50                	push   %eax
       5ea:	9c                   	pushf
       5eb:	33 c0                	xor    %ax,%ax
       5ed:	2e ff 16 14 14       	call   *%cs:0x1414
       5f2:	9c                   	pushf
       5f3:	58                   	pop    %ax
       5f4:	67 81 24 24 ff fc    	andw   $0xfcff,(%esp)
       5fa:	25 00 02             	and    $0x200,%ax
       5fd:	67 09 04 24          	or     %ax,(%esp)
       601:	36 8e 1e 04 02       	mov    %ss:0x204,%ds
       606:	f6 06 87 00 01       	testb  $0x1,0x87
       60b:	36 8e 06 ee 01       	mov    %ss:0x1ee,%es
       610:	0f 85 23 00          	jne    0x637
       614:	36 8e 1e ee 01       	mov    %ss:0x1ee,%ds
       619:	9d                   	popf
       61a:	66 58                	pop    %eax
       61c:	cd 21                	int    $0x21
       61e:	66 0f b7 e4          	movzwl %sp,%esp
       622:	66 50                	push   %eax
       624:	9c                   	pushf
       625:	b8 01 00             	mov    $0x1,%ax
       628:	2e ff 16 16 14       	call   *%cs:0x1416
       62d:	9d                   	popf
       62e:	66 58                	pop    %eax
       630:	0f a9                	pop    %gs
       632:	0f a1                	pop    %fs
       634:	07                   	pop    %es
       635:	1f                   	pop    %ds
       636:	c3                   	ret
       637:	36 8e 1e 1c 02       	mov    %ss:0x21c,%ds
       63c:	66 a1 84 00          	mov    0x84,%eax
       640:	36 8e 1e ee 01       	mov    %ss:0x1ee,%ds
       645:	9d                   	popf
       646:	68 54 16             	push   $0x1654
       649:	66 50                	push   %eax
       64b:	9c                   	pushf
       64c:	0e                   	push   %cs
       64d:	66 58                	pop    %eax
       64f:	67 66 87 44 24 06    	xchg   %eax,0x6(%esp)
       655:	fa                   	cli
       656:	cb                   	lret
       657:	66 1e                	pushl  %ds
       659:	66 50                	push   %eax
       65b:	8c d0                	mov    %ss,%ax
       65d:	9c                   	pushf
       65e:	2e 3b 06 12 10       	cmp    %cs:0x1012,%ax
       663:	0f 84 28 00          	je     0x68f
       667:	9d                   	popf
       668:	16                   	push   %ss
       669:	1f                   	pop    %ds
       66a:	66 8b c4             	mov    %esp,%eax
       66d:	2e 8e 16 12 10       	mov    %cs:0x1012,%ss
       672:	36 66 8b 26 14 01    	mov    %ss:0x114,%esp
       678:	1e                   	push   %ds
       679:	66 50                	push   %eax
       67b:	67 66 c5 00          	lds    (%eax),%eax
       67f:	e8 5c ff             	call   0x5de
       682:	67 66 0f b2 24 24    	lss    (%esp),%esp
       688:	67 66 8d 64 24 08    	lea    0x8(%esp),%esp
       68e:	c3                   	ret
       68f:	9d                   	popf
       690:	66 58                	pop    %eax
       692:	66 1f                	popl   %ds
       694:	e9 47 ff             	jmp    0x5de
       697:	90                   	nop
       698:	88 17                	mov    %dl,(%bx)
       69a:	30 1b                	xor    %bl,(%bp,%di)
       69c:	30 1b                	xor    %bl,(%bp,%di)
       69e:	30 1b                	xor    %bl,(%bp,%di)
       6a0:	30 1b                	xor    %bl,(%bp,%di)
       6a2:	30 1b                	xor    %bl,(%bp,%di)
       6a4:	30 1b                	xor    %bl,(%bp,%di)
       6a6:	30 1b                	xor    %bl,(%bp,%di)
       6a8:	30 1b                	xor    %bl,(%bp,%di)
       6aa:	1f                   	pop    %ds
       6ab:	1a 30                	sbb    (%bx,%si),%dh
       6ad:	1b 30                	sbb    (%bx,%si),%si
       6af:	1b 30                	sbb    (%bx,%si),%si
       6b1:	1b 30                	sbb    (%bx,%si),%si
       6b3:	1b 30                	sbb    (%bx,%si),%si
       6b5:	1b 30                	sbb    (%bx,%si),%si
       6b7:	1b 30                	sbb    (%bx,%si),%si
       6b9:	1b 30                	sbb    (%bx,%si),%si
       6bb:	1b 30                	sbb    (%bx,%si),%si
       6bd:	1b 30                	sbb    (%bx,%si),%si
       6bf:	1b 30                	sbb    (%bx,%si),%si
       6c1:	1b 30                	sbb    (%bx,%si),%si
       6c3:	1b 30                	sbb    (%bx,%si),%si
       6c5:	1b 30                	sbb    (%bx,%si),%si
       6c7:	1b 30                	sbb    (%bx,%si),%si
       6c9:	1b 30                	sbb    (%bx,%si),%si
       6cb:	1b 92 1a 30          	sbb    0x301a(%bp,%si),%dx
       6cf:	1b 30                	sbb    (%bx,%si),%si
       6d1:	1b 30                	sbb    (%bx,%si),%si
       6d3:	1b 30                	sbb    (%bx,%si),%si
       6d5:	1b 30                	sbb    (%bx,%si),%si
       6d7:	1b 30                	sbb    (%bx,%si),%si
       6d9:	1b 30                	sbb    (%bx,%si),%si
       6db:	1b 30                	sbb    (%bx,%si),%si
       6dd:	1b 30                	sbb    (%bx,%si),%si
       6df:	1b 30                	sbb    (%bx,%si),%si
       6e1:	1b 40 27             	sbb    0x27(%bx,%si),%ax
       6e4:	30 1b                	xor    %bl,(%bp,%di)
       6e6:	30 1b                	xor    %bl,(%bp,%di)
       6e8:	30 1b                	xor    %bl,(%bp,%di)
       6ea:	30 1b                	xor    %bl,(%bp,%di)
       6ec:	30 1b                	xor    %bl,(%bp,%di)
       6ee:	30 1b                	xor    %bl,(%bp,%di)
       6f0:	30 1b                	xor    %bl,(%bp,%di)
       6f2:	30 1b                	xor    %bl,(%bp,%di)
       6f4:	30 1b                	xor    %bl,(%bp,%di)
       6f6:	b8 19 30             	mov    $0x3019,%ax
       6f9:	1b 83 1d 30          	sbb    0x301d(%bp,%di),%ax
       6fd:	1b 30                	sbb    (%bx,%si),%si
       6ff:	1b 30                	sbb    (%bx,%si),%si
       701:	1b 7a 27             	sbb    0x27(%bp,%si),%di
       704:	30 1b                	xor    %bl,(%bp,%di)
       706:	30 1b                	xor    %bl,(%bp,%di)
       708:	30 1b                	xor    %bl,(%bp,%di)
       70a:	26 1b 26 1b 26       	sbb    %es:0x261b,%sp
       70f:	1b 26 1b 26          	sbb    0x261b,%sp
       713:	1b 30                	sbb    (%bx,%si),%si
       715:	1b de                	sbb    %si,%bx
       717:	18 49 18             	sbb    %cl,0x18(%bx,%di)
       71a:	26 1b 30             	sbb    %es:(%bx,%si),%si
       71d:	1b 26 1b 30          	sbb    0x301b,%sp
       721:	1b 30                	sbb    (%bx,%si),%si
       723:	1b 30                	sbb    (%bx,%si),%si
       725:	1b 55 1a             	sbb    0x1a(%di),%dx
       728:	8f                   	(bad)
       729:	1c 30                	sbb    $0x30,%al
       72b:	1b 30                	sbb    (%bx,%si),%si
       72d:	1b 58 1b             	sbb    0x1b(%bx,%si),%bx
       730:	88 17                	mov    %dl,(%bx)
       732:	30 1b                	xor    %bl,(%bp,%di)
       734:	d0 19                	rcrb   $1,(%bx,%di)
       736:	da 19                	ficompl (%bx,%di)
       738:	30 1b                	xor    %bl,(%bp,%di)
       73a:	30 1b                	xor    %bl,(%bp,%di)
       73c:	30 1b                	xor    %bl,(%bp,%di)
       73e:	30 1b                	xor    %bl,(%bp,%di)
       740:	30 1b                	xor    %bl,(%bp,%di)
       742:	30 1b                	xor    %bl,(%bp,%di)
       744:	12 1b                	adc    (%bp,%di),%bl
       746:	30 1b                	xor    %bl,(%bp,%di)
       748:	30 1b                	xor    %bl,(%bp,%di)
       74a:	30 1b                	xor    %bl,(%bp,%di)
       74c:	26 1b 26 1b 6a       	sbb    %es:0x6a1b,%sp
       751:	01 66 5b             	add    %sp,0x5b(%bp)
       754:	2e a1 12 10          	mov    %cs:0x1012,%ax
       758:	8e d8                	mov    %ax,%ds
       75a:	8e c0                	mov    %ax,%es
       75c:	8e e0                	mov    %ax,%fs
       75e:	8e e8                	mov    %ax,%gs
       760:	66 53                	push   %ebx
       762:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
       767:	0f 84 61 00          	je     0x7cc
       76b:	fa                   	cli
       76c:	80 3e fc 04 c4       	cmpb   $0xc4,0x4fc
       771:	0f 84 6f 00          	je     0x7e4
       775:	66 a1 08 01          	mov    0x108,%eax
       779:	0f 22 c0             	mov    %eax,%cr0
       77c:	ea b7 17 6f 00       	ljmp   $0x6f,$0x17b7
       781:	0f 22 dc             	mov    %esp,%cr3
       784:	fa                   	cli
       785:	36 0f 01 1e 02 01    	lidtw  %ss:0x102
       78b:	68 00 00             	push   $0x0
       78e:	17                   	pop    %ss
       78f:	68 00 00             	push   $0x0
       792:	1f                   	pop    %ds
       793:	e8 df 25             	call   0x2d75
       796:	2e 80 3e 8e 2e 23    	cmpb   $0x23,%cs:0x2e8e
       79c:	0f 84 25 00          	je     0x7c5
       7a0:	2e 80 3e 8e 2e 24    	cmpb   $0x24,%cs:0x2e8e
       7a6:	0f 84 1d 00          	je     0x7c7
       7aa:	66 33 c0             	xor    %eax,%eax
       7ad:	66 33 db             	xor    %ebx,%ebx
       7b0:	66 33 c9             	xor    %ecx,%ecx
       7b3:	66 33 d2             	xor    %edx,%edx
       7b6:	66 33 f6             	xor    %esi,%esi
       7b9:	66 33 ff             	xor    %edi,%edi
       7bc:	66 33 ed             	xor    %ebp,%ebp
       7bf:	66 58                	pop    %eax
       7c1:	b4 4c                	mov    $0x4c,%ah
       7c3:	cd 21                	int    $0x21
       7c5:	f9                   	stc
       7c6:	cb                   	lret
       7c7:	07                   	pop    %es
       7c8:	1f                   	pop    %ds
       7c9:	66 61                	popal
       7cb:	cf                   	iret
       7cc:	2e a1 12 10          	mov    %cs:0x1012,%ax
       7d0:	8e d8                	mov    %ax,%ds
       7d2:	8e d0                	mov    %ax,%ss
       7d4:	36 66 8b 26 14 01    	mov    %ss:0x114,%esp
       7da:	66 53                	push   %ebx
       7dc:	e8 38 04             	call   0xc17
       7df:	e8 b4 32             	call   0x3a96
       7e2:	eb c6                	jmp    0x7aa
       7e4:	2e 8e 1e 14 10       	mov    %cs:0x1014,%ds
       7e9:	66 33 c0             	xor    %eax,%eax
       7ec:	b8 00 00             	mov    $0x0,%ax
       7ef:	66 0f b7 dc          	movzwl %sp,%ebx
       7f3:	66 50                	push   %eax
       7f5:	66 50                	push   %eax
       7f7:	66 50                	push   %eax
       7f9:	66 53                	push   %ebx
       7fb:	66 50                	push   %eax
       7fd:	66 68 6f 00 00 00    	pushl  $0x6f
       803:	66 68 c1 17 00 00    	pushl  $0x17c1
       809:	b8 0c de             	mov    $0xde0c,%ax
       80c:	66 2e ff 1e 22 10    	lcalll *%cs:0x1022
       812:	f4                   	hlt
       813:	66 83 c4 04          	add    $0x4,%esp
       817:	66 55                	push   %ebp
       819:	66 57                	push   %edi
       81b:	66 56                	push   %esi
       81d:	66 53                	push   %ebx
       81f:	66 33 ed             	xor    %ebp,%ebp
       822:	2e 8e 06 14 10       	mov    %cs:0x1014,%es
       827:	2e 8e 26 12 10       	mov    %cs:0x1012,%fs
       82c:	66 8b f2             	mov    %edx,%esi
       82f:	66 8b d1             	mov    %ecx,%edx
       832:	64 66 8b 3e f0 01    	mov    %fs:0x1f0,%edi
       838:	64 66 3b 0e e4 06    	cmp    %fs:0x6e4,%ecx
       83e:	0f 86 06 00          	jbe    0x848
       842:	64 66 8b 0e e4 06    	mov    %fs:0x6e4,%ecx
       848:	66 2b d1             	sub    %ecx,%edx
       84b:	51                   	push   %cx
       84c:	51                   	push   %cx
       84d:	c1 e9 02             	shr    $0x2,%cx
       850:	67 f3 66 a5          	rep movsl %ds:(%esi),%es:(%edi)
       854:	59                   	pop    %cx
       855:	83 e1 03             	and    $0x3,%cx
       858:	67 f3 a4             	rep movsb %ds:(%esi),%es:(%edi)
       85b:	66 8b fa             	mov    %edx,%edi
       85e:	8b d1                	mov    %cx,%dx
       860:	59                   	pop    %cx
       861:	b4 40                	mov    $0x40,%ah
       863:	2e ff 16 12 16       	call   *%cs:0x1612
       868:	66 0f b7 c0          	movzwl %ax,%eax
       86c:	0f 82 15 00          	jb     0x885
       870:	66 03 e8             	add    %eax,%ebp
       873:	66 8b cf             	mov    %edi,%ecx
       876:	66 0b c9             	or     %ecx,%ecx
       879:	0f 84 08 00          	je     0x885
       87d:	64 3b 06 e4 06       	cmp    %fs:0x6e4,%ax
       882:	74 ab                	je     0x82f
       884:	f8                   	clc
       885:	66 d1 db             	rcr    $1,%ebx
       888:	66 0b ed             	or     %ebp,%ebp
       88b:	0f 84 06 00          	je     0x895
       88f:	66 8b c5             	mov    %ebp,%eax
       892:	66 33 db             	xor    %ebx,%ebx
       895:	66 d1 d3             	rcl    $1,%ebx
       898:	66 5b                	pop    %ebx
       89a:	66 5e                	pop    %esi
       89c:	66 5f                	pop    %edi
       89e:	66 5d                	pop    %ebp
       8a0:	0f a9                	pop    %gs
       8a2:	0f a1                	pop    %fs
       8a4:	07                   	pop    %es
       8a5:	1f                   	pop    %ds
       8a6:	66 cb                	lretl
       8a8:	66 83 c4 04          	add    $0x4,%esp
       8ac:	66 55                	push   %ebp
       8ae:	66 57                	push   %edi
       8b0:	66 56                	push   %esi
       8b2:	66 53                	push   %ebx
       8b4:	66 33 ed             	xor    %ebp,%ebp
       8b7:	1e                   	push   %ds
       8b8:	07                   	pop    %es
       8b9:	2e 8e 1e 14 10       	mov    %cs:0x1014,%ds
       8be:	2e 8e 26 12 10       	mov    %cs:0x1012,%fs
       8c3:	66 8b fa             	mov    %edx,%edi
       8c6:	33 d2                	xor    %dx,%dx
       8c8:	66 8b f1             	mov    %ecx,%esi
       8cb:	64 66 3b 0e e4 06    	cmp    %fs:0x6e4,%ecx
       8d1:	0f 86 06 00          	jbe    0x8db
       8d5:	64 66 8b 0e e4 06    	mov    %fs:0x6e4,%ecx
       8db:	66 2b f1             	sub    %ecx,%esi
       8de:	b4 3f                	mov    $0x3f,%ah
       8e0:	2e ff 16 12 16       	call   *%cs:0x1612
       8e5:	66 0f b7 c0          	movzwl %ax,%eax
       8e9:	0f 82 37 00          	jb     0x924
       8ed:	66 03 e8             	add    %eax,%ebp
       8f0:	66 8b c8             	mov    %eax,%ecx
       8f3:	66 8b d6             	mov    %esi,%edx
       8f6:	64 66 8b 36 f0 01    	mov    %fs:0x1f0,%esi
       8fc:	66 51                	push   %ecx
       8fe:	66 c1 e9 02          	shr    $0x2,%ecx
       902:	67 f3 66 a5          	rep movsl %ds:(%esi),%es:(%edi)
       906:	66 59                	pop    %ecx
       908:	80 e1 03             	and    $0x3,%cl
       90b:	66 0f b6 c9          	movzbl %cl,%ecx
       90f:	67 f3 a4             	rep movsb %ds:(%esi),%es:(%edi)
       912:	66 87 ca             	xchg   %ecx,%edx
       915:	66 0b c9             	or     %ecx,%ecx
       918:	0f 84 08 00          	je     0x924
       91c:	64 3b 06 e4 06       	cmp    %fs:0x6e4,%ax
       921:	74 a5                	je     0x8c8
       923:	f8                   	clc
       924:	66 d1 db             	rcr    $1,%ebx
       927:	66 0b ed             	or     %ebp,%ebp
       92a:	0f 84 06 00          	je     0x934
       92e:	66 8b c5             	mov    %ebp,%eax
       931:	66 33 db             	xor    %ebx,%ebx
       934:	66 d1 d3             	rcl    $1,%ebx
       937:	66 5b                	pop    %ebx
       939:	66 5e                	pop    %esi
       93b:	66 5f                	pop    %edi
       93d:	66 5d                	pop    %ebp
       93f:	0f a9                	pop    %gs
       941:	0f a1                	pop    %fs
       943:	07                   	pop    %es
       944:	1f                   	pop    %ds
       945:	66 cb                	lretl
       947:	66 0f b7 e4          	movzwl %sp,%esp
       94b:	67 ff 74 24 08       	push   0x8(%esp)
       950:	9d                   	popf
       951:	67 66 ff 74 24 04    	pushl  0x4(%esp)
       957:	67 66 8f 44 24 08    	popl   0x8(%esp)
       95d:	67 66 8f 04 24       	popl   (%esp)
       962:	1e                   	push   %ds
       963:	06                   	push   %es
       964:	0f a0                	push   %fs
       966:	0f a8                	push   %gs
       968:	66 50                	push   %eax
       96a:	9c                   	pushf
       96b:	66 0f b6 c4          	movzbl %ah,%eax
       96f:	3c 5b                	cmp    $0x5b,%al
       971:	77 0b                	ja     0x97e
       973:	d1 e0                	shl    $1,%ax
       975:	9d                   	popf
       976:	67 2e ff a0 ce 16 00 	jmp    *%cs:0x16ce(%eax)
       97d:	00 
       97e:	9d                   	popf
       97f:	e9 78 01             	jmp    0xafa
       982:	66 58                	pop    %eax
       984:	0f a9                	pop    %gs
       986:	0f a1                	pop    %fs
       988:	07                   	pop    %es
       989:	1f                   	pop    %ds
       98a:	2e 8e 06 18 10       	mov    %cs:0x1018,%es
       98f:	26 67 66 c4 1d 8a bf 	addr32 les %es:0xbf8a,%ebx
       996:	00 00 
       998:	66 cb                	lretl
       99a:	66 57                	push   %edi
       99c:	66 33 ff             	xor    %edi,%edi
       99f:	e8 08 01             	call   0xaaa
       9a2:	66 5f                	pop    %edi
       9a4:	66 58                	pop    %eax
       9a6:	2e ff 16 12 16       	call   *%cs:0x1612
       9ab:	9c                   	pushf
       9ac:	66 51                	push   %ecx
       9ae:	66 56                	push   %esi
       9b0:	66 57                	push   %edi
       9b2:	b1 2b                	mov    $0x2b,%cl
       9b4:	66 0f b6 c9          	movzbl %cl,%ecx
       9b8:	2e 8e 1e 18 10       	mov    %cs:0x1018,%ds
       9bd:	67 66 c4 3d 8a bf 00 	addr32 les 0xbf8a,%edi
       9c4:	00 
       9c5:	2e 8e 26 12 10       	mov    %cs:0x1012,%fs
       9ca:	64 66 8b 36 f0 01    	mov    %fs:0x1f0,%esi
       9d0:	66 81 c6 a0 0f 00 00 	add    $0xfa0,%esi
       9d7:	2e 8e 1e 14 10       	mov    %cs:0x1014,%ds
       9dc:	67 f3 a4             	rep movsb %ds:(%esi),%es:(%edi)
       9df:	66 5f                	pop    %edi
       9e1:	66 5e                	pop    %esi
       9e3:	66 59                	pop    %ecx
       9e5:	9d                   	popf
       9e6:	e9 18 01             	jmp    0xb01
       9e9:	66 56                	push   %esi
       9eb:	66 57                	push   %edi
       9ed:	66 51                	push   %ecx
       9ef:	2e 8e 26 12 10       	mov    %cs:0x1012,%fs
       9f4:	2e 8e 06 14 10       	mov    %cs:0x1014,%es
       9f9:	64 66 8b 3e f0 01    	mov    %fs:0x1f0,%edi
       9ff:	66 8b f2             	mov    %edx,%esi
       a02:	66 b9 00 10 00 00    	mov    $0x1000,%ecx
       a08:	67 80 3e 24          	cmpb   $0x24,(%esi)
       a0c:	67 a4                	movsb  %ds:(%esi),%es:(%edi)
       a0e:	0f 84 02 00          	je     0xa14
       a12:	e2 f4                	loop   0xa08
       a14:	66 59                	pop    %ecx
       a16:	66 5f                	pop    %edi
       a18:	66 5e                	pop    %esi
       a1a:	33 d2                	xor    %dx,%dx
       a1c:	e9 db 00             	jmp    0xafa
       a1f:	66 58                	pop    %eax
       a21:	66 57                	push   %edi
       a23:	66 56                	push   %esi
       a25:	66 56                	push   %esi
       a27:	33 f6                	xor    %si,%si
       a29:	2e ff 16 12 16       	call   *%cs:0x1612
       a2e:	66 5f                	pop    %edi
       a30:	72 23                	jb     0xa55
       a32:	1e                   	push   %ds
       a33:	07                   	pop    %es
       a34:	2e 8e 26 12 10       	mov    %cs:0x1012,%fs
       a39:	2e 8e 1e 14 10       	mov    %cs:0x1014,%ds
       a3e:	64 66 8b 36 f0 01    	mov    %fs:0x1f0,%esi
       a44:	9c                   	pushf
       a45:	51                   	push   %cx
       a46:	b9 00 10             	mov    $0x1000,%cx
       a49:	67 80 3e 00          	cmpb   $0x0,(%esi)
       a4d:	67 a4                	movsb  %ds:(%esi),%es:(%edi)
       a4f:	74 02                	je     0xa53
       a51:	e2 f6                	loop   0xa49
       a53:	59                   	pop    %cx
       a54:	9d                   	popf
       a55:	66 5e                	pop    %esi
       a57:	66 5f                	pop    %edi
       a59:	e9 a5 00             	jmp    0xb01
       a5c:	66 51                	push   %ecx
       a5e:	66 56                	push   %esi
       a60:	66 57                	push   %edi
       a62:	1e                   	push   %ds
       a63:	1e                   	push   %ds
       a64:	66 8b f2             	mov    %edx,%esi
       a67:	66 b9 0b 00 00 00    	mov    $0xb,%ecx
       a6d:	2e 8e 06 14 10       	mov    %cs:0x1014,%es
       a72:	2e 8e 26 12 10       	mov    %cs:0x1012,%fs
       a77:	64 66 8b 3e f0 01    	mov    %fs:0x1f0,%edi
       a7d:	67 66 8d bf a0 0f 00 	lea    0xfa0(%edi),%edi
       a84:	00 
       a85:	67 f3 66 a5          	rep movsl %ds:(%esi),%es:(%edi)
       a89:	2e 8e 1e 18 10       	mov    %cs:0x1018,%ds
       a8e:	67 66 89 15 8a bf 00 	addr32 mov %edx,0xbf8a
       a95:	00 
       a96:	67 8f 05 8e bf 00 00 	addr32 pop 0xbf8e
       a9d:	1f                   	pop    %ds
       a9e:	66 5f                	pop    %edi
       aa0:	66 5e                	pop    %esi
       aa2:	66 59                	pop    %ecx
       aa4:	ba a0 0f             	mov    $0xfa0,%dx
       aa7:	eb 51                	jmp    0xafa
       aa9:	90                   	nop
       aaa:	2e 8e 26 12 10       	mov    %cs:0x1012,%fs
       aaf:	66 56                	push   %esi
       ab1:	66 57                	push   %edi
       ab3:	66 51                	push   %ecx
       ab5:	66 8b f2             	mov    %edx,%esi
       ab8:	2e 8e 06 14 10       	mov    %cs:0x1014,%es
       abd:	8b d7                	mov    %di,%dx
       abf:	64 66 03 3e f0 01    	add    %fs:0x1f0,%edi
       ac5:	66 b9 d0 07 00 00    	mov    $0x7d0,%ecx
       acb:	67 80 3e 00          	cmpb   $0x0,(%esi)
       acf:	67 a4                	movsb  %ds:(%esi),%es:(%edi)
       ad1:	74 02                	je     0xad5
       ad3:	e2 f6                	loop   0xacb
       ad5:	66 59                	pop    %ecx
       ad7:	66 5f                	pop    %edi
       ad9:	66 5e                	pop    %esi
       adb:	c3                   	ret
       adc:	66 52                	push   %edx
       ade:	66 8b d7             	mov    %edi,%edx
       ae1:	66 bf d0 07 00 00    	mov    $0x7d0,%edi
       ae7:	66 57                	push   %edi
       ae9:	e8 be ff             	call   0xaaa
       aec:	66 5f                	pop    %edi
       aee:	66 5a                	pop    %edx
       af0:	66 57                	push   %edi
       af2:	66 33 ff             	xor    %edi,%edi
       af5:	e8 b2 ff             	call   0xaaa
       af8:	66 5f                	pop    %edi
       afa:	66 58                	pop    %eax
       afc:	2e ff 16 12 16       	call   *%cs:0x1612
       b01:	0f a9                	pop    %gs
       b03:	0f a1                	pop    %fs
       b05:	07                   	pop    %es
       b06:	1f                   	pop    %ds
       b07:	66 0f b7 c0          	movzwl %ax,%eax
       b0b:	66 cb                	lretl
       b0d:	00 00                	add    %al,(%bx,%si)
       b0f:	00 00                	add    %al,(%bx,%si)
       b11:	00 00                	add    %al,(%bx,%si)
       b13:	e8 7c 02             	call   0xd92
       b16:	64 81 2e 14 01 80 01 	subw   $0x180,%fs:0x114
       b1d:	e8 67 2f             	call   0x3a87
       b20:	eb 0f                	jmp    0xb31
       b22:	66 58                	pop    %eax
       b24:	2e 8e 26 12 10       	mov    %cs:0x1012,%fs
       b29:	64 80 3e fc 04 c0    	cmpb   $0xc0,%fs:0x4fc
       b2f:	74 e2                	je     0xb13
       b31:	66 56                	push   %esi
       b33:	66 57                	push   %edi
       b35:	66 53                	push   %ebx
       b37:	66 52                	push   %edx
       b39:	66 51                	push   %ecx
       b3b:	06                   	push   %es
       b3c:	66 33 ff             	xor    %edi,%edi
       b3f:	e8 68 ff             	call   0xaaa
       b42:	0f a9                	pop    %gs
       b44:	64 66 8b 3e f0 01    	mov    %fs:0x1f0,%edi
       b4a:	67 66 8d bf 80 00 00 	lea    0x80(%edi),%edi
       b51:	00 
       b52:	65 67 66 c5 33       	lds    %gs:(%ebx),%esi
       b57:	66 b9 80 0e 00 00    	mov    $0xe80,%ecx
       b5d:	67 ac                	lods   %ds:(%esi),%al
       b5f:	67 aa                	stos   %al,%es:(%edi)
       b61:	0b c0                	or     %ax,%ax
       b63:	8a e0                	mov    %al,%ah
       b65:	e0 f6                	loopne 0xb5d
       b67:	66 03 f9             	add    %ecx,%edi
       b6a:	65 67 66 c5 73 06    	lds    %gs:0x6(%ebx),%esi
       b70:	67 66 0f b6 0e       	movzbl (%esi),%ecx
       b75:	83 c1 02             	add    $0x2,%cx
       b78:	67 f3 a4             	rep movsb %ds:(%esi),%es:(%edi)
       b7b:	64 ff 36 7c 01       	push   %fs:0x17c
       b80:	6a 6c                	push   $0x6c
       b82:	64 ff 36 7c 01       	push   %fs:0x17c
       b87:	6a 5c                	push   $0x5c
       b89:	64 ff 36 ee 01       	push   %fs:0x1ee
       b8e:	68 00 0f             	push   $0xf00
       b91:	64 a1 ee 01          	mov    %fs:0x1ee,%ax
       b95:	05 08 00             	add    $0x8,%ax
       b98:	50                   	push   %ax
       b99:	e8 7b 00             	call   0xc17
       b9c:	b0 00                	mov    $0x0,%al
       b9e:	2e ff 16 14 14       	call   *%cs:0x1414
       ba3:	e8 0f 23             	call   0x2eb5
       ba6:	b8 00 4b             	mov    $0x4b00,%ax
       ba9:	8b dc                	mov    %sp,%bx
       bab:	16                   	push   %ss
       bac:	07                   	pop    %es
       bad:	36 8e 1e ee 01       	mov    %ss:0x1ee,%ds
       bb2:	33 d2                	xor    %dx,%dx
       bb4:	2e 89 26 43 1b       	mov    %sp,%cs:0x1b43
       bb9:	cd 21                	int    $0x21
       bbb:	2e 66 0f b2 26 43 1b 	lss    %cs:0x1b43,%esp
       bc2:	8b f0                	mov    %ax,%si
       bc4:	66 60                	pushal
       bc6:	9c                   	pushf
       bc7:	16                   	push   %ss
       bc8:	1f                   	pop    %ds
       bc9:	e8 b1 23             	call   0x2f7d
       bcc:	9d                   	popf
       bcd:	66 61                	popal
       bcf:	67 66 8d 64 24 0e    	lea    0xe(%esp),%esp
       bd5:	9c                   	pushf
       bd6:	b0 01                	mov    $0x1,%al
       bd8:	2e ff 16 16 14       	call   *%cs:0x1416
       bdd:	67 81 24 24 ff 3f    	andw   $0x3fff,(%esp)
       be3:	67 81 0c 24 00 30    	orw    $0x3000,(%esp)
       be9:	e8 50 00             	call   0xc3c
       bec:	9d                   	popf
       bed:	66 59                	pop    %ecx
       bef:	66 5a                	pop    %edx
       bf1:	66 5b                	pop    %ebx
       bf3:	66 5f                	pop    %edi
       bf5:	8b c6                	mov    %si,%ax
       bf7:	66 5e                	pop    %esi
       bf9:	9c                   	pushf
       bfa:	36 80 3e fc 04 c0    	cmpb   $0xc0,%ss:0x4fc
       c00:	74 04                	je     0xc06
       c02:	9d                   	popf
       c03:	e9 fb fe             	jmp    0xb01
       c06:	36 81 06 14 01 80 01 	addw   $0x180,%ss:0x114
       c0d:	e8 77 2e             	call   0x3a87
       c10:	9d                   	popf
       c11:	e8 a5 01             	call   0xdb9
       c14:	e9 ea fe             	jmp    0xb01
       c17:	06                   	push   %es
       c18:	2e 8e 06 12 10       	mov    %cs:0x1012,%es
       c1d:	26 80 3e f8 01 00    	cmpb   $0x0,%es:0x1f8
       c23:	75 02                	jne    0xc27
       c25:	07                   	pop    %es
       c26:	c3                   	ret
       c27:	66 50                	push   %eax
       c29:	26 a1 0c 01          	mov    %es:0x10c,%ax
       c2d:	26 a3 f6 01          	mov    %ax,%es:0x1f6
       c31:	26 a1 08 01          	mov    %es:0x108,%ax
       c35:	e8 76 0a             	call   0x16ae
       c38:	66 58                	pop    %eax
       c3a:	eb e9                	jmp    0xc25
       c3c:	06                   	push   %es
       c3d:	2e 8e 06 12 10       	mov    %cs:0x1012,%es
       c42:	26 80 3e f8 01 00    	cmpb   $0x0,%es:0x1f8
       c48:	75 02                	jne    0xc4c
       c4a:	07                   	pop    %es
       c4b:	c3                   	ret
       c4c:	66 50                	push   %eax
       c4e:	26 a1 f6 01          	mov    %es:0x1f6,%ax
       c52:	e8 59 0a             	call   0x16ae
       c55:	66 58                	pop    %eax
       c57:	eb f1                	jmp    0xc4a
       c59:	66 58                	pop    %eax
       c5b:	66 52                	push   %edx
       c5d:	66 0f b7 d3          	movzwl %bx,%edx
       c61:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
       c66:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
       c6b:	0f 84 b0 00          	je     0xd1f
       c6f:	81 3e f6 04 57 3c    	cmpw   $0x3c57,0x4f6
       c75:	0f 87 a6 00          	ja     0xd1f
       c79:	66 a1 f8 04          	mov    0x4f8,%eax
       c7d:	74 04                	je     0xc83
       c7f:	66 a1 c0 04          	mov    0x4c0,%eax
       c83:	66 8b 1e c4 04       	mov    0x4c4,%ebx
       c88:	66 2b d8             	sub    %eax,%ebx
       c8b:	0f 86 90 00          	jbe    0xd1f
       c8f:	66 52                	push   %edx
       c91:	66 c1 e2 04          	shl    $0x4,%edx
       c95:	67 66 8d 82 ff 0f 00 	lea    0xfff(%edx),%eax
       c9c:	00 
       c9d:	25 00 f0             	and    $0xf000,%ax
       ca0:	66 3b da             	cmp    %edx,%ebx
       ca3:	66 5a                	pop    %edx
       ca5:	72 2d                	jb     0xcd4
       ca7:	e8 1c 0d             	call   0x19c6
       caa:	0f 82 26 00          	jb     0xcd4
       cae:	81 3e f6 04 57 3c    	cmpw   $0x3c57,0x4f6
       cb4:	0f 84 7f 00          	je     0xd37
       cb8:	66 8b 1e c0 04       	mov    0x4c0,%ebx
       cbd:	66 01 06 c0 04       	add    %eax,0x4c0
       cc2:	66 8b c3             	mov    %ebx,%eax
       cc5:	66 2b 1e cc 04       	sub    0x4cc,%ebx
       cca:	66 c1 e8 04          	shr    $0x4,%eax
       cce:	f8                   	clc
       ccf:	66 5a                	pop    %edx
       cd1:	e9 2d fe             	jmp    0xb01
       cd4:	66 8b 1e c4 04       	mov    0x4c4,%ebx
       cd9:	66 2b 1e c0 04       	sub    0x4c0,%ebx
       cde:	66 ff 36 ec 04       	pushl  0x4ec
       ce3:	66 ff 36 d8 04       	pushl  0x4d8
       ce8:	66 01 1e ec 04       	add    %ebx,0x4ec
       ced:	66 51                	push   %ecx
       cef:	66 52                	push   %edx
       cf1:	e8 ea 1d             	call   0x2ade
       cf4:	66 0b c0             	or     %eax,%eax
       cf7:	75 12                	jne    0xd0b
       cf9:	66 81 2e ec 04 00 10 	subl   $0x1000,0x4ec
       d00:	00 00 
       d02:	66 81 eb 00 10 00 00 	sub    $0x1000,%ebx
       d09:	75 e6                	jne    0xcf1
       d0b:	66 5a                	pop    %edx
       d0d:	66 59                	pop    %ecx
       d0f:	66 c1 eb 04          	shr    $0x4,%ebx
       d13:	66 8f 06 d8 04       	popl   0x4d8
       d18:	66 8f 06 ec 04       	popl   0x4ec
       d1d:	eb 03                	jmp    0xd22
       d1f:	66 33 db             	xor    %ebx,%ebx
       d22:	87 d3                	xchg   %dx,%bx
       d24:	b4 48                	mov    $0x48,%ah
       d26:	2e ff 16 12 16       	call   *%cs:0x1612
       d2b:	72 17                	jb     0xd44
       d2d:	66 0f b7 d8          	movzwl %ax,%ebx
       d31:	66 c1 e3 04          	shl    $0x4,%ebx
       d35:	eb 8b                	jmp    0xcc2
       d37:	66 8b 1e f8 04       	mov    0x4f8,%ebx
       d3c:	66 01 06 f8 04       	add    %eax,0x4f8
       d41:	e9 7e ff             	jmp    0xcc2
       d44:	3b da                	cmp    %dx,%bx
       d46:	77 02                	ja     0xd4a
       d48:	8b da                	mov    %dx,%bx
       d4a:	f9                   	stc
       d4b:	eb 82                	jmp    0xccf
       d4d:	58                   	pop    %ax
       d4e:	2e 8e 16 12 10       	mov    %cs:0x1012,%ss
       d53:	36 66 8b 26 78 01    	mov    %ss:0x178,%esp
       d59:	36 66 89 26 14 01    	mov    %esp,%ss:0x114
       d5f:	36 80 3e fc 04 c0    	cmpb   $0xc0,%ss:0x4fc
       d65:	74 1d                	je     0xd84
       d67:	2e 8e 1e 16 10       	mov    %cs:0x1016,%ds
       d6c:	66 33 f6             	xor    %esi,%esi
       d6f:	36 66 8b 0e d0 04    	mov    %ss:0x4d0,%ecx
       d75:	66 c1 e9 0c          	shr    $0xc,%ecx
       d79:	67 ac                	lods   %ds:(%esi),%al
       d7b:	66 81 c6 ff 0f 00 00 	add    $0xfff,%esi
       d82:	e2 f5                	loop   0xd79
       d84:	50                   	push   %ax
       d85:	b0 00                	mov    $0x0,%al
       d87:	2e ff 16 14 14       	call   *%cs:0x1414
       d8c:	e8 26 21             	call   0x2eb5
       d8f:	58                   	pop    %ax
       d90:	cd 21                	int    $0x21
       d92:	1e                   	push   %ds
       d93:	66 50                	push   %eax
       d95:	9c                   	pushf
       d96:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
       d9b:	81 2e 14 01 80 01    	subw   $0x180,0x114
       da1:	66 a1 14 01          	mov    0x114,%eax
       da5:	9d                   	popf
       da6:	1e                   	push   %ds
       da7:	16                   	push   %ss
       da8:	1f                   	pop    %ds
       da9:	17                   	pop    %ss
       daa:	66 94                	xchg   %eax,%esp
       dac:	66 1e                	pushl  %ds
       dae:	66 50                	push   %eax
       db0:	67 ff 70 06          	push   0x6(%eax)
       db4:	67 66 c5 00          	lds    (%eax),%eax
       db8:	c3                   	ret
       db9:	1e                   	push   %ds
       dba:	66 50                	push   %eax
       dbc:	67 66 c5 44 24 08    	lds    0x8(%esp),%eax
       dc2:	67 66 8f 00          	popl   (%eax)
       dc6:	67 66 8f 40 04       	popl   0x4(%eax)
       dcb:	1e                   	push   %ds
       dcc:	16                   	push   %ss
       dcd:	1f                   	pop    %ds
       dce:	17                   	pop    %ss
       dcf:	66 8b e0             	mov    %eax,%esp
       dd2:	66 58                	pop    %eax
       dd4:	9c                   	pushf
       dd5:	81 06 14 01 80 01    	addw   $0x180,0x114
       ddb:	9d                   	popf
       ddc:	1f                   	pop    %ds
       ddd:	c3                   	ret
       dde:	80 1e 91 1e 13       	sbbb   $0x13,0x1e91
       de3:	1f                   	pop    %ds
       de4:	42                   	inc    %dx
       de5:	1f                   	pop    %ds
       de6:	9e                   	sahf
       de7:	1f                   	pop    %ds
       de8:	3c 21                	cmp    $0x21,%al
       dea:	e1 21                	loope  0xe0d
       dec:	67 22 96 22 f0 22 80 	and    -0x7fdd0fde(%esi),%dl
       df3:	1e                   	push   %ds
       df4:	80 1e 2b 23 3a       	sbbb   $0x3a,0x232b
       df9:	23 63 23             	and    0x23(%bp,%di),%sp
       dfc:	76 24                	jbe    0xe22
       dfe:	80 1e f5 24 80       	sbbb   $0x80,0x24f5
       e03:	1e                   	push   %ds
       e04:	80 1e 80 1e 80       	sbbb   $0x80,0x1e80
       e09:	1e                   	push   %ds
       e0a:	80 1e 80 1e 80       	sbbb   $0x80,0x1e80
       e0f:	1e                   	push   %ds
       e10:	80 1e 80 1e 80       	sbbb   $0x80,0x1e80
       e15:	1e                   	push   %ds
       e16:	80 1e 80 1e 80       	sbbb   $0x80,0x1e80
       e1b:	1e                   	push   %ds
       e1c:	80 1e 80 1e 80       	sbbb   $0x80,0x1e80
       e21:	1e                   	push   %ds
       e22:	80 1e 80 1e 80       	sbbb   $0x80,0x1e80
       e27:	1e                   	push   %ds
       e28:	80 1e 80 1e 80       	sbbb   $0x80,0x1e80
       e2d:	1e                   	push   %ds
       e2e:	80 1e 80 1e 80       	sbbb   $0x80,0x1e80
       e33:	1e                   	push   %ds
       e34:	75 25                	jne    0xe5b
       e36:	80 1e 80 1e 80       	sbbb   $0x80,0x1e80
       e3b:	1e                   	push   %ds
       e3c:	80 1e 80 1e 80       	sbbb   $0x80,0x1e80
       e41:	1e                   	push   %ds
       e42:	1e                   	push   %ds
       e43:	26 57                	es push %di
       e45:	26 80 1e a5 26 66    	sbbb   $0x66,%es:0x26a5
       e4b:	58                   	pop    %ax
       e4c:	66 b8 a5 a5 a5 a5    	mov    $0xa5a5a5a5,%eax
       e52:	0f a9                	pop    %gs
       e54:	0f a1                	pop    %fs
       e56:	07                   	pop    %es
       e57:	1f                   	pop    %ds
       e58:	f9                   	stc
       e59:	66 cb                	lretl
       e5b:	66 51                	push   %ecx
       e5d:	66 57                	push   %edi
       e5f:	2e 8e 06 18 10       	mov    %cs:0x1018,%es
       e64:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
       e69:	66 8b 3e 10 02       	mov    0x210,%edi
       e6e:	b9 00 01             	mov    $0x100,%cx
       e71:	33 c0                	xor    %ax,%ax
       e73:	67 66 8d 7f 06       	lea    0x6(%edi),%edi
       e78:	fc                   	cld
       e79:	67 ab                	stos   %ax,%es:(%edi)
       e7b:	e2 f6                	loop   0xe73
       e7d:	66 5f                	pop    %edi
       e7f:	66 59                	pop    %ecx
       e81:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
       e86:	75 18                	jne    0xea0
       e88:	a1 28 00             	mov    0x28,%ax
       e8b:	a3 28 00             	mov    %ax,0x28
       e8e:	8c d0                	mov    %ss,%ax
       e90:	2e 3b 06 12 10       	cmp    %cs:0x1012,%ax
       e95:	74 43                	je     0xeda
       e97:	2e 3b 06 14 10       	cmp    %cs:0x1014,%ax
       e9c:	74 3c                	je     0xeda
       e9e:	eb 30                	jmp    0xed0
       ea0:	67 8b 44 24 18       	mov    0x18(%esp),%ax
       ea5:	26 67 3b 05 82 bf 00 	addr32 cmp %es:0xbf82,%ax
       eac:	00 
       ead:	74 2b                	je     0xeda
       eaf:	2e 3b 06 14 10       	cmp    %cs:0x1014,%ax
       eb4:	74 24                	je     0xeda
       eb6:	56                   	push   %si
       eb7:	57                   	push   %di
       eb8:	51                   	push   %cx
       eb9:	8b f4                	mov    %sp,%si
       ebb:	1e                   	push   %ds
       ebc:	07                   	pop    %es
       ebd:	8b 3e 78 01          	mov    0x178,%di
       ec1:	83 ef 22             	sub    $0x22,%di
       ec4:	b9 11 00             	mov    $0x11,%cx
       ec7:	fa                   	cli
       ec8:	8b e7                	mov    %di,%sp
       eca:	f3 a5                	rep movsw %ds:(%si),%es:(%di)
       ecc:	fb                   	sti
       ecd:	59                   	pop    %cx
       ece:	5f                   	pop    %di
       ecf:	5e                   	pop    %si
       ed0:	66 a1 78 01          	mov    0x178,%eax
       ed4:	66 a3 14 01          	mov    %eax,0x114
       ed8:	eb 27                	jmp    0xf01
       eda:	e9 d2 03             	jmp    0x12af
       edd:	2e 8e 1e 18 10       	mov    %cs:0x1018,%ds
       ee2:	2e 8e 06 12 10       	mov    %cs:0x1012,%es
       ee7:	66 0f b6 d9          	movzbl %cl,%ebx
       eeb:	c1 e3 03             	shl    $0x3,%bx
       eee:	26 66 03 1e 10 02    	add    %es:0x210,%ebx
       ef4:	67 8b 43 04          	mov    0x4(%ebx),%ax
       ef8:	67 66 8b 1b          	mov    (%ebx),%ebx
       efc:	67 89 44 24 08       	mov    %ax,0x8(%esp)
       f01:	66 58                	pop    %eax
       f03:	0f a9                	pop    %gs
       f05:	0f a1                	pop    %fs
       f07:	07                   	pop    %es
       f08:	1f                   	pop    %ds
       f09:	f8                   	clc
       f0a:	66 cb                	lretl
       f0c:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
       f11:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
       f16:	74 1c                	je     0xf34
       f18:	80 f9 24             	cmp    $0x24,%cl
       f1b:	74 44                	je     0xf61
       f1d:	83 ec 08             	sub    $0x8,%sp
       f20:	68 21 00             	push   $0x21
       f23:	b4 35                	mov    $0x35,%ah
       f25:	8a c1                	mov    %cl,%al
       f27:	e8 9a f3             	call   0x2c4
       f2a:	83 c4 04             	add    $0x4,%sp
       f2d:	53                   	push   %bx
       f2e:	66 5b                	pop    %ebx
       f30:	66 58                	pop    %eax
       f32:	eb cd                	jmp    0xf01
       f34:	b8 00 02             	mov    $0x200,%ax
       f37:	51                   	push   %cx
       f38:	52                   	push   %dx
       f39:	8a d9                	mov    %cl,%bl
       f3b:	66 56                	push   %esi
       f3d:	66 57                	push   %edi
       f3f:	66 8b f4             	mov    %esp,%esi
       f42:	8c d7                	mov    %ss,%di
       f44:	2e 8e 16 12 10       	mov    %cs:0x1012,%ss
       f49:	66 8b 26 14 01       	mov    0x114,%esp
       f4e:	cd 31                	int    $0x31
       f50:	8e d7                	mov    %di,%ss
       f52:	66 8b e6             	mov    %esi,%esp
       f55:	66 5f                	pop    %edi
       f57:	66 5e                	pop    %esi
       f59:	51                   	push   %cx
       f5a:	52                   	push   %dx
       f5b:	66 5b                	pop    %ebx
       f5d:	5a                   	pop    %dx
       f5e:	59                   	pop    %cx
       f5f:	eb a0                	jmp    0xf01
       f61:	66 8b 1e 00 02       	mov    0x200,%ebx
       f66:	eb 99                	jmp    0xf01
       f68:	66 53                	push   %ebx
       f6a:	66 51                	push   %ecx
       f6c:	1e                   	push   %ds
       f6d:	2e 8e 1e 18 10       	mov    %cs:0x1018,%ds
       f72:	2e 8e 06 12 10       	mov    %cs:0x1012,%es
       f77:	66 0f b6 c1          	movzbl %cl,%eax
       f7b:	c1 e0 03             	shl    $0x3,%ax
       f7e:	26 66 03 06 10 02    	add    %es:0x210,%eax
       f84:	67 8f 40 04          	pop    0x4(%eax)
       f88:	67 66 89 10          	mov    %edx,(%eax)
       f8c:	66 0f b6 d9          	movzbl %cl,%ebx
       f90:	26 66 0f b7 06 04 02 	movzwl %es:0x204,%eax
       f97:	66 c1 e0 04          	shl    $0x4,%eax
       f9b:	2e 8e 26 14 10       	mov    %cs:0x1014,%fs
       fa0:	64 67 80 4c 98 03 01 	orb    $0x1,%fs:0x3(%eax,%ebx,4)
       fa7:	26 66 3b 16 48 01    	cmp    %es:0x148,%edx
       fad:	72 0f                	jb     0xfbe
       faf:	26 66 3b 16 10 02    	cmp    %es:0x210,%edx
       fb5:	77 07                	ja     0xfbe
       fb7:	64 67 80 64 98 03 fe 	andb   $0xfe,%fs:0x3(%eax,%ebx,4)
       fbe:	80 e1 f8             	and    $0xf8,%cl
       fc1:	80 f9 08             	cmp    $0x8,%cl
       fc4:	74 2e                	je     0xff4
       fc6:	80 f9 70             	cmp    $0x70,%cl
       fc9:	74 35                	je     0x1000
       fcb:	26 3a 0e 1e 02       	cmp    %es:0x21e,%cl
       fd0:	74 3d                	je     0x100f
       fd2:	26 3a 0e 1f 02       	cmp    %es:0x21f,%cl
       fd7:	74 3f                	je     0x1018
       fd9:	8c c8                	mov    %cs,%ax
       fdb:	66 b9 64 35 00 00    	mov    $0x3564,%ecx
       fe1:	26 66 3b 16 48 01    	cmp    %es:0x148,%edx
       fe7:	72 63                	jb     0x104c
       fe9:	26 66 3b 16 10 02    	cmp    %es:0x210,%edx
       fef:	72 3e                	jb     0x102f
       ff1:	f9                   	stc
       ff2:	eb 58                	jmp    0x104c
       ff4:	80 eb 08             	sub    $0x8,%bl
       ff7:	8b c3                	mov    %bx,%ax
       ff9:	26 02 1e 1e 02       	add    %es:0x21e,%bl
       ffe:	eb 21                	jmp    0x1021
      1000:	80 eb 68             	sub    $0x68,%bl
      1003:	8b c3                	mov    %bx,%ax
      1005:	26 02 1e 1f 02       	add    %es:0x21f,%bl
      100a:	80 eb 08             	sub    $0x8,%bl
      100d:	eb 12                	jmp    0x1021
      100f:	8b c3                	mov    %bx,%ax
      1011:	26 2a 06 1e 02       	sub    %es:0x21e,%al
      1016:	eb 09                	jmp    0x1021
      1018:	8b c3                	mov    %bx,%ax
      101a:	26 2a 06 1f 02       	sub    %es:0x21f,%al
      101f:	04 08                	add    $0x8,%al
      1021:	c1 e0 02             	shl    $0x2,%ax
      1024:	05 09 38             	add    $0x3809,%ax
      1027:	66 0f b7 c8          	movzwl %ax,%ecx
      102b:	8c c8                	mov    %cs,%ax
      102d:	eb b2                	jmp    0xfe1
      102f:	67 66 8d 42 01       	lea    0x1(%edx),%eax
      1034:	26 66 2b 06 08 02    	sub    %es:0x208,%eax
      103a:	c1 e0 02             	shl    $0x2,%ax
      103d:	26 66 03 06 14 02    	add    %es:0x214,%eax
      1043:	67 66 8b 08          	mov    (%eax),%ecx
      1047:	67 8b 40 04          	mov    0x4(%eax),%ax
      104b:	f8                   	clc
      104c:	53                   	push   %bx
      104d:	9c                   	pushf
      104e:	80 fb 0f             	cmp    $0xf,%bl
      1051:	76 34                	jbe    0x1087
      1053:	5b                   	pop    %bx
      1054:	5b                   	pop    %bx
      1055:	26 80 3e fc 04 c0    	cmpb   $0xc0,%es:0x4fc
      105b:	74 43                	je     0x10a0
      105d:	80 fb 0f             	cmp    $0xf,%bl
      1060:	0f 86 1c 00          	jbe    0x1080
      1064:	66 c1 e3 03          	shl    $0x3,%ebx
      1068:	26 66 03 1e fe 00    	add    %es:0xfe,%ebx
      106e:	2e 8e 1e 14 10       	mov    %cs:0x1014,%ds
      1073:	66 51                	push   %ecx
      1075:	67 8f 03             	pop    (%ebx)
      1078:	67 8f 43 06          	pop    0x6(%ebx)
      107c:	67 89 43 02          	mov    %ax,0x2(%ebx)
      1080:	66 59                	pop    %ecx
      1082:	66 5b                	pop    %ebx
      1084:	e9 7a fe             	jmp    0xf01
      1087:	80 fb 07             	cmp    $0x7,%bl
      108a:	76 03                	jbe    0x108f
      108c:	80 c3 08             	add    $0x8,%bl
      108f:	26 0f ab 1e fc 01    	bts    %bx,%es:0x1fc
      1095:	9d                   	popf
      1096:	72 bc                	jb     0x1054
      1098:	26 0f b3 1e fc 01    	btr    %bx,%es:0x1fc
      109e:	eb b4                	jmp    0x1054
      10a0:	66 56                	push   %esi
      10a2:	66 57                	push   %edi
      10a4:	66 52                	push   %edx
      10a6:	66 8b d1             	mov    %ecx,%edx
      10a9:	66 81 f9 64 35 00 00 	cmp    $0x3564,%ecx
      10b0:	0f 85 2d 00          	jne    0x10e1
      10b4:	66 ba 49 38 00 00    	mov    $0x3849,%edx
      10ba:	80 fb 1c             	cmp    $0x1c,%bl
      10bd:	0f 84 20 00          	je     0x10e1
      10c1:	66 ba 4d 38 00 00    	mov    $0x384d,%edx
      10c7:	80 fb 23             	cmp    $0x23,%bl
      10ca:	0f 84 13 00          	je     0x10e1
      10ce:	66 ba 52 38 00 00    	mov    $0x3852,%edx
      10d4:	80 fb 24             	cmp    $0x24,%bl
      10d7:	0f 84 06 00          	je     0x10e1
      10db:	66 ba 16 36 00 00    	mov    $0x3616,%edx
      10e1:	8b c8                	mov    %ax,%cx
      10e3:	b8 05 02             	mov    $0x205,%ax
      10e6:	66 8b f4             	mov    %esp,%esi
      10e9:	8c d7                	mov    %ss,%di
      10eb:	2e 8e 16 12 10       	mov    %cs:0x1012,%ss
      10f0:	26 66 8b 26 14 01    	mov    %es:0x114,%esp
      10f6:	cd 31                	int    $0x31
      10f8:	8e d7                	mov    %di,%ss
      10fa:	66 8b e6             	mov    %esi,%esp
      10fd:	66 5a                	pop    %edx
      10ff:	66 5f                	pop    %edi
      1101:	66 5e                	pop    %esi
      1103:	e9 7a ff             	jmp    0x1080
      1106:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      110b:	2e 8e 06 18 10       	mov    %cs:0x1018,%es
      1110:	66 0f b6 c1          	movzbl %cl,%eax
      1114:	66 c1 e0 03          	shl    $0x3,%eax
      1118:	66 03 06 10 02       	add    0x210,%eax
      111d:	26 67 66 8b 00       	mov    %es:(%eax),%eax
      1122:	66 3b 06 48 01       	cmp    0x148,%eax
      1127:	0f 82 20 00          	jb     0x114b
      112b:	66 3b 06 10 02       	cmp    0x210,%eax
      1130:	0f 87 17 00          	ja     0x114b
      1134:	66 0f b6 c1          	movzbl %cl,%eax
      1138:	66 c1 e0 02          	shl    $0x2,%eax
      113c:	66 03 06 18 02       	add    0x218,%eax
      1141:	2e 8e 06 14 10       	mov    %cs:0x1014,%es
      1146:	26 67 66 89 18       	mov    %ebx,%es:(%eax)
      114b:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      1150:	74 21                	je     0x1173
      1152:	80 f9 24             	cmp    $0x24,%cl
      1155:	74 4c                	je     0x11a3
      1157:	83 ec 06             	sub    $0x6,%sp
      115a:	66 53                	push   %ebx
      115c:	87 d3                	xchg   %dx,%bx
      115e:	67 c7 04 24 21 00    	movw   $0x21,(%esp)
      1164:	b4 25                	mov    $0x25,%ah
      1166:	8a c1                	mov    %cl,%al
      1168:	e8 59 f1             	call   0x2c4
      116b:	83 c4 0a             	add    $0xa,%sp
      116e:	87 d3                	xchg   %dx,%bx
      1170:	e9 8e fd             	jmp    0xf01
      1173:	b8 01 02             	mov    $0x201,%ax
      1176:	53                   	push   %bx
      1177:	51                   	push   %cx
      1178:	52                   	push   %dx
      1179:	66 53                	push   %ebx
      117b:	8a d9                	mov    %cl,%bl
      117d:	5a                   	pop    %dx
      117e:	59                   	pop    %cx
      117f:	66 56                	push   %esi
      1181:	66 57                	push   %edi
      1183:	66 8b f4             	mov    %esp,%esi
      1186:	8c d7                	mov    %ss,%di
      1188:	2e 8e 16 12 10       	mov    %cs:0x1012,%ss
      118d:	66 8b 26 14 01       	mov    0x114,%esp
      1192:	cd 31                	int    $0x31
      1194:	8e d7                	mov    %di,%ss
      1196:	66 8b e6             	mov    %esi,%esp
      1199:	66 5f                	pop    %edi
      119b:	66 5e                	pop    %esi
      119d:	5a                   	pop    %dx
      119e:	59                   	pop    %cx
      119f:	5b                   	pop    %bx
      11a0:	e9 5e fd             	jmp    0xf01
      11a3:	66 89 1e 00 02       	mov    %ebx,0x200
      11a8:	e9 56 fd             	jmp    0xf01
      11ab:	66 0e                	pushl  %cs
      11ad:	66 68 f5 21 00 00    	pushl  $0x21f5
      11b3:	1e                   	push   %ds
      11b4:	06                   	push   %es
      11b5:	0f a0                	push   %fs
      11b7:	0f a8                	push   %gs
      11b9:	66 50                	push   %eax
      11bb:	fa                   	cli
      11bc:	e9 a9 fd             	jmp    0xf68
      11bf:	2e 8e 06 12 10       	mov    %cs:0x1012,%es
      11c4:	66 53                	push   %ebx
      11c6:	66 51                	push   %ecx
      11c8:	26 80 3e fc 04 c0    	cmpb   $0xc0,%es:0x4fc
      11ce:	0f 85 2a 00          	jne    0x11fc
      11d2:	80 f9 1c             	cmp    $0x1c,%cl
      11d5:	0f 84 50 00          	je     0x1229
      11d9:	80 f9 23             	cmp    $0x23,%cl
      11dc:	0f 84 49 00          	je     0x1229
      11e0:	80 f9 24             	cmp    $0x24,%cl
      11e3:	0f 84 42 00          	je     0x1229
      11e7:	80 e1 f8             	and    $0xf8,%cl
      11ea:	26 3a 0e 1e 02       	cmp    %es:0x21e,%cl
      11ef:	0f 84 36 00          	je     0x1229
      11f3:	26 3a 0e 1f 02       	cmp    %es:0x21f,%cl
      11f8:	0f 84 2d 00          	je     0x1229
      11fc:	66 59                	pop    %ecx
      11fe:	bb 6f 00             	mov    $0x6f,%bx
      1201:	66 c1 e3 10          	shl    $0x10,%ebx
      1205:	0f b6 d9             	movzbw %cl,%bx
      1208:	c1 e3 02             	shl    $0x2,%bx
      120b:	26 03 1e 06 02       	add    %es:0x206,%bx
      1210:	66 0e                	pushl  %cs
      1212:	66 68 59 22 00 00    	pushl  $0x2259
      1218:	1e                   	push   %ds
      1219:	06                   	push   %es
      121a:	0f a0                	push   %fs
      121c:	0f a8                	push   %gs
      121e:	66 50                	push   %eax
      1220:	e9 e3 fe             	jmp    0x1106
      1223:	66 5b                	pop    %ebx
      1225:	fb                   	sti
      1226:	e9 d8 fc             	jmp    0xf01
      1229:	66 59                	pop    %ecx
      122b:	66 5b                	pop    %ebx
      122d:	fb                   	sti
      122e:	e9 d0 fc             	jmp    0xf01
      1231:	66 53                	push   %ebx
      1233:	66 0e                	pushl  %cs
      1235:	66 68 7d 22 00 00    	pushl  $0x227d
      123b:	1e                   	push   %ds
      123c:	06                   	push   %es
      123d:	0f a0                	push   %fs
      123f:	0f a8                	push   %gs
      1241:	66 50                	push   %eax
      1243:	fa                   	cli
      1244:	e9 bf fe             	jmp    0x1106
      1247:	66 5b                	pop    %ebx
      1249:	66 0e                	pushl  %cs
      124b:	66 68 92 22 00 00    	pushl  $0x2292
      1251:	1e                   	push   %ds
      1252:	06                   	push   %es
      1253:	0f a0                	push   %fs
      1255:	0f a8                	push   %gs
      1257:	66 50                	push   %eax
      1259:	e9 0c fd             	jmp    0xf68
      125c:	fb                   	sti
      125d:	e9 a1 fc             	jmp    0xf01
      1260:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      1265:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      126a:	74 17                	je     0x1283
      126c:	e8 d9 0b             	call   0x1e48
      126f:	72 3e                	jb     0x12af
      1271:	67 8a 68 07          	mov    0x7(%eax),%ch
      1275:	67 8a 48 04          	mov    0x4(%eax),%cl
      1279:	51                   	push   %cx
      127a:	67 ff 70 02          	push   0x2(%eax)
      127e:	66 59                	pop    %ecx
      1280:	e9 7e fc             	jmp    0xf01
      1283:	b8 06 00             	mov    $0x6,%ax
      1286:	52                   	push   %dx
      1287:	66 56                	push   %esi
      1289:	66 57                	push   %edi
      128b:	66 8b f4             	mov    %esp,%esi
      128e:	8c d7                	mov    %ss,%di
      1290:	2e 8e 16 12 10       	mov    %cs:0x1012,%ss
      1295:	66 8b 26 14 01       	mov    0x114,%esp
      129a:	cd 31                	int    $0x31
      129c:	8e d7                	mov    %di,%ss
      129e:	66 8b e6             	mov    %esi,%esp
      12a1:	66 5f                	pop    %edi
      12a3:	66 5e                	pop    %esi
      12a5:	51                   	push   %cx
      12a6:	52                   	push   %dx
      12a7:	66 59                	pop    %ecx
      12a9:	5a                   	pop    %dx
      12aa:	72 03                	jb     0x12af
      12ac:	e9 52 fc             	jmp    0xf01
      12af:	f9                   	stc
      12b0:	66 58                	pop    %eax
      12b2:	0f a9                	pop    %gs
      12b4:	0f a1                	pop    %fs
      12b6:	07                   	pop    %es
      12b7:	1f                   	pop    %ds
      12b8:	66 cb                	lretl
      12ba:	66 58                	pop    %eax
      12bc:	2e a1 18 10          	mov    %cs:0x1018,%ax
      12c0:	8e d8                	mov    %ax,%ds
      12c2:	50                   	push   %ax
      12c3:	67 ff 35 82 bf 00 00 	addr32 push 0xbf82
      12ca:	0e                   	push   %cs
      12cb:	68 6f 00             	push   $0x6f
      12ce:	66 5b                	pop    %ebx
      12d0:	2e ff 36 12 10       	push   %cs:0x1012
      12d5:	68 00 00             	push   $0x0
      12d8:	66 59                	pop    %ecx
      12da:	2e ff 36 14 10       	push   %cs:0x1014
      12df:	2e ff 36 1a 10       	push   %cs:0x101a
      12e4:	66 5a                	pop    %edx
      12e6:	2e ff 36 1c 10       	push   %cs:0x101c
      12eb:	2e ff 36 1e 10       	push   %cs:0x101e
      12f0:	66 5e                	pop    %esi
      12f2:	e9 0c fc             	jmp    0xf01
      12f5:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      12fa:	58                   	pop    %ax
      12fb:	ff 36 1e 02          	push   0x21e
      12ff:	33 db                	xor    %bx,%bx
      1301:	e9 fd fb             	jmp    0xf01
      1304:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      1309:	66 58                	pop    %eax
      130b:	68 6f 00             	push   $0x6f
      130e:	68 79 11             	push   $0x1179
      1311:	66 8b 1e ec 01       	mov    0x1ec,%ebx
      1316:	66 8b 0e e4 06       	mov    0x6e4,%ecx
      131b:	2e 8b 16 14 10       	mov    %cs:0x1014,%dx
      1320:	67 89 54 24 08       	mov    %dx,0x8(%esp)
      1325:	66 8b 16 f0 01       	mov    0x1f0,%edx
      132a:	e9 d4 fb             	jmp    0xf01
      132d:	67 66 8b 04 24       	mov    (%esp),%eax
      1332:	9c                   	pushf
      1333:	66 f7 c1 c0 ff ff ff 	test   $0xffffffc0,%ecx
      133a:	0f 85 f5 00          	jne    0x1433
      133e:	51                   	push   %cx
      133f:	66 59                	pop    %ecx
      1341:	2e 8e 26 12 10       	mov    %cs:0x1012,%fs
      1346:	2e 8e 1e 18 10       	mov    %cs:0x1018,%ds
      134b:	64 80 3e fc 04 c0    	cmpb   $0xc0,%fs:0x4fc
      1351:	0f 84 94 00          	je     0x13e9
      1355:	67 66 ff 74 24 18    	pushl  0x18(%esp)
      135b:	67 66 ff 74 24 18    	pushl  0x18(%esp)
      1361:	66 50                	push   %eax
      1363:	67 8b 44 24 08       	mov    0x8(%esp),%ax
      1368:	67 3b 05 82 bf 00 00 	addr32 cmp 0xbf82,%ax
      136f:	75 39                	jne    0x13aa
      1371:	67 66 c5 44 24 04    	lds    0x4(%esp),%eax
      1377:	67 66 81 78 f4 15 bf 	cmpl   $0xbf15,-0xc(%eax)
      137e:	00 00 
      1380:	75 28                	jne    0x13aa
      1382:	67 66 8b 00          	mov    (%eax),%eax
      1386:	64 66 3b 06 78 01    	cmp    %fs:0x178,%eax
      138c:	73 1c                	jae    0x13aa
      138e:	66 50                	push   %eax
      1390:	67 66 0f 02 40 20    	lar    0x20(%eax),%eax
      1396:	66 58                	pop    %eax
      1398:	75 10                	jne    0x13aa
      139a:	67 66 c5 40 1c       	lds    0x1c(%eax),%eax
      139f:	67 66 89 44 24 04    	mov    %eax,0x4(%esp)
      13a5:	67 8c 5c 24 08       	mov    %ds,0x8(%esp)
      13aa:	66 58                	pop    %eax
      13ac:	66 53                	push   %ebx
      13ae:	66 c1 cb 10          	ror    $0x10,%ebx
      13b2:	66 53                	push   %ebx
      13b4:	66 53                	push   %ebx
      13b6:	66 c1 cb 10          	ror    $0x10,%ebx
      13ba:	51                   	push   %cx
      13bb:	c1 e1 08             	shl    $0x8,%cx
      13be:	80 cd 80             	or     $0x80,%ch
      13c1:	67 87 0c 24          	xchg   %cx,(%esp)
      13c5:	66 51                	push   %ecx
      13c7:	59                   	pop    %cx
      13c8:	66 0f b7 c9          	movzwl %cx,%ecx
      13cc:	9d                   	popf
      13cd:	e8 f4 ee             	call   0x2c4
      13d0:	9c                   	pushf
      13d1:	64 80 3e fc 04 c0    	cmpb   $0xc0,%fs:0x4fc
      13d7:	74 36                	je     0x140f
      13d9:	67 66 89 44 24 18    	mov    %eax,0x18(%esp)
      13df:	58                   	pop    %ax
      13e0:	66 83 c4 16          	add    $0x16,%esp
      13e4:	50                   	push   %ax
      13e5:	9d                   	popf
      13e6:	e9 18 fb             	jmp    0xf01
      13e9:	66 58                	pop    %eax
      13eb:	66 56                	push   %esi
      13ed:	67 66 8d 74 24 14    	lea    0x14(%esp),%esi
      13f3:	16                   	push   %ss
      13f4:	0f a9                	pop    %gs
      13f6:	2e 8e 16 12 10       	mov    %cs:0x1012,%ss
      13fb:	64 66 8b 26 14 01    	mov    %fs:0x114,%esp
      1401:	66 0f a8             	pushl  %gs
      1404:	66 56                	push   %esi
      1406:	65 67 66 8b 76 ec    	mov    %gs:-0x14(%esi),%esi
      140c:	e9 52 ff             	jmp    0x1361
      140f:	66 56                	push   %esi
      1411:	66 50                	push   %eax
      1413:	16                   	push   %ss
      1414:	1f                   	pop    %ds
      1415:	66 8b f4             	mov    %esp,%esi
      1418:	67 66 8b 46 18       	mov    0x18(%esi),%eax
      141d:	67 8e 56 1c          	mov    0x1c(%esi),%ss
      1421:	67 66 8d 60 f0       	lea    -0x10(%eax),%esp
      1426:	66 ad                	lods   %ds:(%si),%eax
      1428:	66 50                	push   %eax
      142a:	66 ad                	lods   %ds:(%si),%eax
      142c:	66 50                	push   %eax
      142e:	ad                   	lods   %ds:(%si),%ax
      142f:	66 5e                	pop    %esi
      1431:	eb b1                	jmp    0x13e4
      1433:	9d                   	popf
      1434:	67 66 c7 04 24 01 00 	movl   $0x1,(%esp)
      143b:	00 00 
      143d:	e9 6f fe             	jmp    0x12af
      1440:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      1445:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      144a:	0f 84 34 00          	je     0x1482
      144e:	66 33 c0             	xor    %eax,%eax
      1451:	8c c0                	mov    %es,%ax
      1453:	24 f8                	and    $0xf8,%al
      1455:	67 66 8b 88 11 00 00 	mov    0x11(%eax),%ecx
      145c:	00 
      145d:	67 8a 88 17 00 00 00 	mov    0x17(%eax),%cl
      1464:	66 c1 c9 08          	ror    $0x8,%ecx
      1468:	66 03 cb             	add    %ebx,%ecx
      146b:	66 c1 c1 0c          	rol    $0xc,%ecx
      146f:	f7 c1 ff 0f          	test   $0xfff,%cx
      1473:	75 06                	jne    0x147b
      1475:	c1 e9 0c             	shr    $0xc,%cx
      1478:	e9 86 fa             	jmp    0xf01
      147b:	66 c1 c9 0c          	ror    $0xc,%ecx
      147f:	e9 2d fe             	jmp    0x12af
      1482:	66 53                	push   %ebx
      1484:	66 52                	push   %edx
      1486:	66 56                	push   %esi
      1488:	66 57                	push   %edi
      148a:	8c c3                	mov    %es,%bx
      148c:	b8 06 00             	mov    $0x6,%ax
      148f:	8c d6                	mov    %ss,%si
      1491:	66 8b fc             	mov    %esp,%edi
      1494:	2e 8e 16 12 10       	mov    %cs:0x1012,%ss
      1499:	36 66 8b 26 14 01    	mov    %ss:0x114,%esp
      149f:	cd 31                	int    $0x31
      14a1:	8e d6                	mov    %si,%ss
      14a3:	66 8b e7             	mov    %edi,%esp
      14a6:	66 5f                	pop    %edi
      14a8:	66 5e                	pop    %esi
      14aa:	9c                   	pushf
      14ab:	66 c1 e1 10          	shl    $0x10,%ecx
      14af:	9d                   	popf
      14b0:	8b ca                	mov    %dx,%cx
      14b2:	66 5a                	pop    %edx
      14b4:	66 5b                	pop    %ebx
      14b6:	0f 82 f5 fd          	jb     0x12af
      14ba:	66 03 cb             	add    %ebx,%ecx
      14bd:	eb ac                	jmp    0x146b
      14bf:	9c                   	pushf
      14c0:	8c d0                	mov    %ss,%ax
      14c2:	2e 3b 06 12 10       	cmp    %cs:0x1012,%ax
      14c7:	0f 85 52 00          	jne    0x151d
      14cb:	9d                   	popf
      14cc:	67 89 04 24          	mov    %ax,(%esp)
      14d0:	67 66 8d 44 24 04    	lea    0x4(%esp),%eax
      14d6:	66 50                	push   %eax
      14d8:	66 52                	push   %edx
      14da:	67 66 ff 72 06       	pushl  0x6(%edx)
      14df:	67 66 ff 72 02       	pushl  0x2(%edx)
      14e4:	67 ff 32             	push   (%edx)
      14e7:	67 66 8b 42 0a       	mov    0xa(%edx),%eax
      14ec:	67 66 8b 52 0e       	mov    0xe(%edx),%edx
      14f1:	e8 d0 ed             	call   0x2c4
      14f4:	67 66 87 54 24 0a    	xchg   %edx,0xa(%esp)
      14fa:	67 66 8d 64 24 02    	lea    0x2(%esp),%esp
      1500:	67 66 8f 42 02       	popl   0x2(%edx)
      1505:	67 66 8f 42 06       	popl   0x6(%edx)
      150a:	67 66 8f 42 0e       	popl   0xe(%edx)
      150f:	67 66 0f b2 24 24    	lss    (%esp),%esp
      1515:	0f a9                	pop    %gs
      1517:	0f a1                	pop    %fs
      1519:	07                   	pop    %es
      151a:	1f                   	pop    %ds
      151b:	66 cb                	lretl
      151d:	9d                   	popf
      151e:	1e                   	push   %ds
      151f:	66 56                	push   %esi
      1521:	16                   	push   %ss
      1522:	1f                   	pop    %ds
      1523:	67 66 8d 74 24 0a    	lea    0xa(%esp),%esi
      1529:	2e 8e 16 12 10       	mov    %cs:0x1012,%ss
      152e:	36 66 8b 26 14 01    	mov    %ss:0x114,%esp
      1534:	66 1e                	pushl  %ds
      1536:	66 56                	push   %esi
      1538:	67 66 c5 76 f6       	lds    -0xa(%esi),%esi
      153d:	eb 99                	jmp    0x14d8
      153f:	66 58                	pop    %eax
      1541:	66 53                	push   %ebx
      1543:	66 51                	push   %ecx
      1545:	66 52                	push   %edx
      1547:	66 57                	push   %edi
      1549:	66 56                	push   %esi
      154b:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      1550:	33 c0                	xor    %ax,%ax
      1552:	fe c7                	inc    %bh
      1554:	d0 df                	rcr    $1,%bh
      1556:	d1 d0                	rcl    $1,%ax
      1558:	80 ff 03             	cmp    $0x3,%bh
      155b:	0f 85 6c 00          	jne    0x15cb
      155f:	80 fb 01             	cmp    $0x1,%bl
      1562:	0f 87 65 00          	ja     0x15cb
      1566:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      156b:	75 48                	jne    0x15b5
      156d:	66 8b f4             	mov    %esp,%esi
      1570:	8c d7                	mov    %ss,%di
      1572:	2e 8e 16 12 10       	mov    %cs:0x1012,%ss
      1577:	66 8b 26 14 01       	mov    0x114,%esp
      157c:	66 57                	push   %edi
      157e:	66 56                	push   %esi
      1580:	66 52                	push   %edx
      1582:	0a db                	or     %bl,%bl
      1584:	0f 84 18 00          	je     0x15a0
      1588:	66 51                	push   %ecx
      158a:	50                   	push   %ax
      158b:	b8 06 00             	mov    $0x6,%ax
      158e:	8c c3                	mov    %es,%bx
      1590:	cd 31                	int    $0x31
      1592:	0f 82 3e 00          	jb     0x15d4
      1596:	58                   	pop    %ax
      1597:	51                   	push   %cx
      1598:	52                   	push   %dx
      1599:	66 5a                	pop    %edx
      159b:	66 59                	pop    %ecx
      159d:	66 03 ca             	add    %edx,%ecx
      15a0:	5f                   	pop    %di
      15a1:	5e                   	pop    %si
      15a2:	b4 06                	mov    $0x6,%ah
      15a4:	66 0f a4 cb 10       	shld   $0x10,%ecx,%ebx
      15a9:	cd 31                	int    $0x31
      15ab:	0f 82 2a 00          	jb     0x15d9
      15af:	67 66 0f b2 24 24    	lss    (%esp),%esp
      15b5:	f8                   	clc
      15b6:	b8 2b 25             	mov    $0x252b,%ax
      15b9:	66 5e                	pop    %esi
      15bb:	66 5f                	pop    %edi
      15bd:	66 5a                	pop    %edx
      15bf:	66 59                	pop    %ecx
      15c1:	66 5b                	pop    %ebx
      15c3:	0f a9                	pop    %gs
      15c5:	0f a1                	pop    %fs
      15c7:	07                   	pop    %es
      15c8:	1f                   	pop    %ds
      15c9:	66 cb                	lretl
      15cb:	66 b8 a5 a5 a5 a5    	mov    $0xa5a5a5a5,%eax
      15d1:	f9                   	stc
      15d2:	eb e5                	jmp    0x15b9
      15d4:	58                   	pop    %ax
      15d5:	66 59                	pop    %ecx
      15d7:	66 5a                	pop    %edx
      15d9:	67 66 0f b2 24 24    	lss    (%esp),%esp
      15df:	f9                   	stc
      15e0:	66 b8 08 00 00 00    	mov    $0x8,%eax
      15e6:	eb d1                	jmp    0x15b9
      15e8:	80 f9 0f             	cmp    $0xf,%cl
      15eb:	77 28                	ja     0x1615
      15ed:	80 f9 08             	cmp    $0x8,%cl
      15f0:	0f 82 e9 f8          	jb     0xedd
      15f4:	66 51                	push   %ecx
      15f6:	66 0f b6 c9          	movzbl %cl,%ecx
      15fa:	2e 8e 1e 18 10       	mov    %cs:0x1018,%ds
      15ff:	67 8e 04 cd f8 be 00 	mov    0xbef8(,%ecx,8),%es
      1606:	00 
      1607:	67 66 8b 1c cd f4 be 	mov    0xbef4(,%ecx,8),%ebx
      160e:	00 00 
      1610:	66 59                	pop    %ecx
      1612:	e9 ec f8             	jmp    0xf01
      1615:	67 66 c7 04 24 81 00 	movl   $0x81,(%esp)
      161c:	00 00 
      161e:	e9 8e fc             	jmp    0x12af
      1621:	80 f9 10             	cmp    $0x10,%cl
      1624:	77 ef                	ja     0x1615
      1626:	80 f9 08             	cmp    $0x8,%cl
      1629:	0f 82 3b f9          	jb     0xf68
      162d:	66 51                	push   %ecx
      162f:	66 52                	push   %edx
      1631:	66 0f b6 c9          	movzbl %cl,%ecx
      1635:	2e 8e 06 18 10       	mov    %cs:0x1018,%es
      163a:	26 67 8c 1c cd f8 be 	mov    %ds,%es:0xbef8(,%ecx,8)
      1641:	00 00 
      1643:	26 67 66 89 14 cd f4 	mov    %edx,%es:0xbef4(,%ecx,8)
      164a:	be 00 00 
      164d:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      1652:	0f b3 0e fc 01       	btr    %cx,0x1fc
      1657:	66 81 ea 1c bf 00 00 	sub    $0xbf1c,%edx
      165e:	66 83 fa 10          	cmp    $0x10,%edx
      1662:	76 05                	jbe    0x1669
      1664:	0f ab 0e fc 01       	bts    %cx,0x1fc
      1669:	66 5a                	pop    %edx
      166b:	66 59                	pop    %ecx
      166d:	eb a3                	jmp    0x1612
      166f:	2e 8e 06 12 10       	mov    %cs:0x1012,%es
      1674:	26 80 3e f8 01 00    	cmpb   $0x0,%es:0x1f8
      167a:	75 1b                	jne    0x1697
      167c:	26 c6 06 f8 01 01    	movb   $0x1,%es:0x1f8
      1682:	26 80 3e fc 04 00    	cmpb   $0x0,%es:0x4fc
      1688:	74 0d                	je     0x1697
      168a:	e8 5e 00             	call   0x16eb
      168d:	26 a2 08 01          	mov    %al,%es:0x108
      1691:	0a db                	or     %bl,%bl
      1693:	0f 84 11 00          	je     0x16a8
      1697:	0a db                	or     %bl,%bl
      1699:	74 0a                	je     0x16a5
      169b:	67 66 0f b6 02       	movzbl (%edx),%eax
      16a0:	e8 0b 00             	call   0x16ae
      16a3:	eb 06                	jmp    0x16ab
      16a5:	e8 43 00             	call   0x16eb
      16a8:	67 88 02             	mov    %al,(%edx)
      16ab:	e9 53 f8             	jmp    0xf01
      16ae:	66 53                	push   %ebx
      16b0:	66 83 e0 06          	and    $0x6,%eax
      16b4:	26 80 3e fc 04 c0    	cmpb   $0xc0,%es:0x4fc
      16ba:	74 13                	je     0x16cf
      16bc:	0f 20 c3             	mov    %cr0,%ebx
      16bf:	80 e3 f9             	and    $0xf9,%bl
      16c2:	66 0b c3             	or     %ebx,%eax
      16c5:	0f 22 c0             	mov    %eax,%cr0
      16c8:	26 a3 0c 01          	mov    %ax,%es:0x10c
      16cc:	66 5b                	pop    %ebx
      16ce:	c3                   	ret
      16cf:	66 50                	push   %eax
      16d1:	26 a3 0c 01          	mov    %ax,%es:0x10c
      16d5:	e8 ba f6             	call   0xd92
      16d8:	8b d8                	mov    %ax,%bx
      16da:	d1 cb                	ror    $1,%bx
      16dc:	b8 01 0e             	mov    $0xe01,%ax
      16df:	cd 31                	int    $0x31
      16e1:	e8 d5 f6             	call   0xdb9
      16e4:	66 58                	pop    %eax
      16e6:	72 d4                	jb     0x16bc
      16e8:	66 5b                	pop    %ebx
      16ea:	c3                   	ret
      16eb:	26 80 3e fc 04 c0    	cmpb   $0xc0,%es:0x4fc
      16f1:	74 06                	je     0x16f9
      16f3:	0f 20 c0             	mov    %cr0,%eax
      16f6:	24 06                	and    $0x6,%al
      16f8:	c3                   	ret
      16f9:	e8 96 f6             	call   0xd92
      16fc:	b8 00 0e             	mov    $0xe00,%ax
      16ff:	cd 31                	int    $0x31
      1701:	e8 b5 f6             	call   0xdb9
      1704:	72 ed                	jb     0x16f3
      1706:	d0 e0                	shl    $1,%al
      1708:	eb ec                	jmp    0x16f6
      170a:	67 66 0f b6 04 24    	movzbl (%esp),%eax
      1710:	9c                   	pushf
      1711:	3c 35                	cmp    $0x35,%al
      1713:	77 0b                	ja     0x1720
      1715:	d0 e0                	shl    $1,%al
      1717:	9d                   	popf
      1718:	67 2e ff a0 14 1e 00 	jmp    *%cs:0x1e14(%eax)
      171f:	00 
      1720:	9d                   	popf
      1721:	e9 26 f7             	jmp    0xe4a
      1724:	80 1e 90 27 fb       	sbbb   $0xfb,0x2790
      1729:	27                   	daa
      172a:	36 28 96 22 7a       	sub    %dl,%ss:0x7a22(%bp)
      172f:	28 80 1e 80          	sub    %al,-0x7fe2(%bx,%si)
      1733:	1e                   	push   %ds
      1734:	f6 28                	imulb  (%bx,%si)
      1736:	b5 29                	mov    $0x29,%ch
      1738:	26 2a 1f             	sub    %es:(%bx),%bl
      173b:	2b 4f 2b             	sub    0x2b(%bx),%cx
      173e:	bb 2b 80             	mov    $0x802b,%bx
      1741:	1e                   	push   %ds
      1742:	80 1e 67 66 0f       	sbbb   $0xf,0x6667
      1747:	b6 04                	mov    $0x4,%dh
      1749:	24 9c                	and    $0x9c,%al
      174b:	3c 0f                	cmp    $0xf,%al
      174d:	77 d1                	ja     0x1720
      174f:	d0 e0                	shl    $1,%al
      1751:	9d                   	popf
      1752:	67 2e ff a0 5a 27 00 	jmp    *%cs:0x275a(%eax)
      1759:	00 
      175a:	66 51                	push   %ecx
      175c:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      1761:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      1766:	0f 84 34 00          	je     0x179e
      176a:	66 b9 08 00 00 00    	mov    $0x8,%ecx
      1770:	67 f6 04 cd 8d 00 00 	testb  $0x4,0x8d(,%ecx,8)
      1777:	00 04 
      1779:	0f 85 0a 00          	jne    0x1787
      177d:	e2 f1                	loop   0x1770
      177f:	66 33 db             	xor    %ebx,%ebx
      1782:	66 59                	pop    %ecx
      1784:	e9 28 fb             	jmp    0x12af
      1787:	67 66 8d 1c cd f8 ff 	lea    -0x8(,%ecx,8),%ebx
      178e:	ff ff 
      1790:	c6 87 95 00 f3       	movb   $0xf3,0x95(%bx)
      1795:	81 c3 80 00          	add    $0x80,%bx
      1799:	66 59                	pop    %ecx
      179b:	e9 74 fe             	jmp    0x1612
      179e:	e8 f1 f5             	call   0xd92
      17a1:	33 c0                	xor    %ax,%ax
      17a3:	b9 01 00             	mov    $0x1,%cx
      17a6:	cd 31                	int    $0x31
      17a8:	72 14                	jb     0x17be
      17aa:	66 0f b7 d8          	movzwl %ax,%ebx
      17ae:	2e 66 0f 02 0e 14 10 	lar    %cs:0x1014,%ecx
      17b5:	66 c1 c9 08          	ror    $0x8,%ecx
      17b9:	b8 09 00             	mov    $0x9,%ax
      17bc:	cd 31                	int    $0x31
      17be:	e8 f8 f5             	call   0xdb9
      17c1:	73 d6                	jae    0x1799
      17c3:	eb ba                	jmp    0x177f
      17c5:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      17ca:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      17cf:	74 1d                	je     0x17ee
      17d1:	81 fb 80 00          	cmp    $0x80,%bx
      17d5:	0f 82 d6 fa          	jb     0x12af
      17d9:	81 fb c3 00          	cmp    $0xc3,%bx
      17dd:	0f 87 ce fa          	ja     0x12af
      17e1:	53                   	push   %bx
      17e2:	80 e3 f8             	and    $0xf8,%bl
      17e5:	80 8f 15 00 04       	orb    $0x4,0x15(%bx)
      17ea:	5b                   	pop    %bx
      17eb:	e9 24 fe             	jmp    0x1612
      17ee:	b8 01 00             	mov    $0x1,%ax
      17f1:	e8 9e f5             	call   0xd92
      17f4:	cd 31                	int    $0x31
      17f6:	e8 c0 f5             	call   0xdb9
      17f9:	0f 82 b2 fa          	jb     0x12af
      17fd:	e9 12 fe             	jmp    0x1612
      1800:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      1805:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      180a:	74 1e                	je     0x182a
      180c:	e8 39 06             	call   0x1e48
      180f:	0f 82 9c fa          	jb     0x12af
      1813:	67 89 48 02          	mov    %cx,0x2(%eax)
      1817:	66 c1 c9 10          	ror    $0x10,%ecx
      181b:	67 88 48 04          	mov    %cl,0x4(%eax)
      181f:	67 88 68 07          	mov    %ch,0x7(%eax)
      1823:	66 c1 c9 10          	ror    $0x10,%ecx
      1827:	e9 e8 fd             	jmp    0x1612
      182a:	51                   	push   %cx
      182b:	52                   	push   %dx
      182c:	b8 07 00             	mov    $0x7,%ax
      182f:	66 51                	push   %ecx
      1831:	5a                   	pop    %dx
      1832:	59                   	pop    %cx
      1833:	e8 5c f5             	call   0xd92
      1836:	cd 31                	int    $0x31
      1838:	e8 7e f5             	call   0xdb9
      183b:	5a                   	pop    %dx
      183c:	59                   	pop    %cx
      183d:	0f 82 6e fa          	jb     0x12af
      1841:	e9 bd f6             	jmp    0xf01
      1844:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      1849:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      184e:	74 3d                	je     0x188d
      1850:	e8 f5 05             	call   0x1e48
      1853:	0f 82 58 fa          	jb     0x12af
      1857:	67 80 60 06 7f       	andb   $0x7f,0x6(%eax)
      185c:	66 81 f9 00 00 10 00 	cmp    $0x100000,%ecx
      1863:	72 0d                	jb     0x1872
      1865:	66 41                	inc    %ecx
      1867:	67 80 48 06 80       	orb    $0x80,0x6(%eax)
      186c:	66 c1 e9 0c          	shr    $0xc,%ecx
      1870:	66 49                	dec    %ecx
      1872:	67 89 08             	mov    %cx,(%eax)
      1875:	66 c1 e9 10          	shr    $0x10,%ecx
      1879:	67 80 60 06 f0       	andb   $0xf0,0x6(%eax)
      187e:	67 08 48 06          	or     %cl,0x6(%eax)
      1882:	53                   	push   %bx
      1883:	67 66 0f 03 0c 24    	lsl    (%esp),%ecx
      1889:	5b                   	pop    %bx
      188a:	e9 85 fd             	jmp    0x1612
      188d:	51                   	push   %cx
      188e:	52                   	push   %dx
      188f:	66 81 f9 00 00 10 00 	cmp    $0x100000,%ecx
      1896:	72 08                	jb     0x18a0
      1898:	66 41                	inc    %ecx
      189a:	81 e1 00 f0          	and    $0xf000,%cx
      189e:	66 49                	dec    %ecx
      18a0:	66 51                	push   %ecx
      18a2:	5a                   	pop    %dx
      18a3:	59                   	pop    %cx
      18a4:	b8 08 00             	mov    $0x8,%ax
      18a7:	e8 e8 f4             	call   0xd92
      18aa:	cd 31                	int    $0x31
      18ac:	e8 0a f5             	call   0xdb9
      18af:	5a                   	pop    %dx
      18b0:	59                   	pop    %cx
      18b1:	0f 82 fa f9          	jb     0x12af
      18b5:	53                   	push   %bx
      18b6:	67 66 0f 03 0c 24    	lsl    (%esp),%ecx
      18bc:	5b                   	pop    %bx
      18bd:	e9 52 fd             	jmp    0x1612
      18c0:	66 81 c1 ff 0f 00 00 	add    $0xfff,%ecx
      18c7:	81 e1 00 f0          	and    $0xf000,%cx
      18cb:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      18d0:	80 3e fd 04 01       	cmpb   $0x1,0x4fd
      18d5:	74 36                	je     0x190d
      18d7:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      18dc:	74 32                	je     0x1910
      18de:	66 2b 0e d0 04       	sub    0x4d0,%ecx
      18e3:	76 0e                	jbe    0x18f3
      18e5:	66 29 0e d8 04       	sub    %ecx,0x4d8
      18ea:	0f 82 1a 00          	jb     0x1908
      18ee:	66 01 0e d0 04       	add    %ecx,0x4d0
      18f3:	66 58                	pop    %eax
      18f5:	66 a1 d4 04          	mov    0x4d4,%eax
      18f9:	66 2b 06 d8 04       	sub    0x4d8,%eax
      18fe:	66 50                	push   %eax
      1900:	66 8b 0e d0 04       	mov    0x4d0,%ecx
      1905:	e9 f9 f5             	jmp    0xf01
      1908:	66 01 0e d8 04       	add    %ecx,0x4d8
      190d:	e9 9f f9             	jmp    0x12af
      1910:	66 3b 0e d0 04       	cmp    0x4d0,%ecx
      1915:	76 05                	jbe    0x191c
      1917:	e8 10 00             	call   0x192a
      191a:	72 f1                	jb     0x190d
      191c:	66 58                	pop    %eax
      191e:	66 ff 36 d4 04       	pushl  0x4d4
      1923:	66 8b 0e d0 04       	mov    0x4d0,%ecx
      1928:	eb db                	jmp    0x1905
      192a:	66 51                	push   %ecx
      192c:	56                   	push   %si
      192d:	57                   	push   %di
      192e:	53                   	push   %bx
      192f:	b8 03 05             	mov    $0x503,%ax
      1932:	8b 36 02 06          	mov    0x602,%si
      1936:	8b 3e 00 06          	mov    0x600,%di
      193a:	66 51                	push   %ecx
      193c:	59                   	pop    %cx
      193d:	5b                   	pop    %bx
      193e:	e8 51 f4             	call   0xd92
      1941:	cd 31                	int    $0x31
      1943:	72 31                	jb     0x1976
      1945:	89 36 02 06          	mov    %si,0x602
      1949:	89 3e 00 06          	mov    %di,0x600
      194d:	53                   	push   %bx
      194e:	51                   	push   %cx
      194f:	66 58                	pop    %eax
      1951:	66 3b 06 04 06       	cmp    0x604,%eax
      1956:	74 03                	je     0x195b
      1958:	e8 19 18             	call   0x3174
      195b:	e8 5b f4             	call   0xdb9
      195e:	5b                   	pop    %bx
      195f:	5f                   	pop    %di
      1960:	5e                   	pop    %si
      1961:	66 59                	pop    %ecx
      1963:	66 a1 d0 04          	mov    0x4d0,%eax
      1967:	66 89 0e d0 04       	mov    %ecx,0x4d0
      196c:	66 2b c1             	sub    %ecx,%eax
      196f:	66 01 06 d8 04       	add    %eax,0x4d8
      1974:	f8                   	clc
      1975:	c3                   	ret
      1976:	e8 40 f4             	call   0xdb9
      1979:	5b                   	pop    %bx
      197a:	5f                   	pop    %di
      197b:	5e                   	pop    %si
      197c:	66 59                	pop    %ecx
      197e:	c3                   	ret
      197f:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      1984:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      1989:	0f 85 04 02          	jne    0x1b91
      198d:	66 51                	push   %ecx
      198f:	66 03 0e d0 04       	add    0x4d0,%ecx
      1994:	66 81 c1 ff 0f 00 00 	add    $0xfff,%ecx
      199b:	81 e1 00 f0          	and    $0xf000,%cx
      199f:	66 51                	push   %ecx
      19a1:	66 ff 36 d0 04       	pushl  0x4d0
      19a6:	e8 81 ff             	call   0x192a
      19a9:	72 13                	jb     0x19be
      19ab:	66 5a                	pop    %edx
      19ad:	66 5b                	pop    %ebx
      19af:	66 89 16 d4 04       	mov    %edx,0x4d4
      19b4:	66 8b f3             	mov    %ebx,%esi
      19b7:	66 83 c4 04          	add    $0x4,%esp
      19bb:	e9 43 f5             	jmp    0xf01
      19be:	66 83 c4 08          	add    $0x8,%esp
      19c2:	66 59                	pop    %ecx
      19c4:	eb 76                	jmp    0x1a3c
      19c6:	66 60                	pushal
      19c8:	66 ff 36 ec 04       	pushl  0x4ec
      19cd:	66 01 06 ec 04       	add    %eax,0x4ec
      19d2:	66 ff 36 d8 04       	pushl  0x4d8
      19d7:	e8 04 11             	call   0x2ade
      19da:	66 0b c0             	or     %eax,%eax
      19dd:	66 5b                	pop    %ebx
      19df:	66 58                	pop    %eax
      19e1:	75 0a                	jne    0x19ed
      19e3:	66 89 1e d8 04       	mov    %ebx,0x4d8
      19e8:	66 a3 ec 04          	mov    %eax,0x4ec
      19ec:	f9                   	stc
      19ed:	66 61                	popal
      19ef:	c3                   	ret
      19f0:	66 56                	push   %esi
      19f2:	66 57                	push   %edi
      19f4:	66 53                	push   %ebx
      19f6:	66 51                	push   %ecx
      19f8:	67 66 8d 79 ff       	lea    -0x1(%ecx),%edi
      19fd:	66 03 fb             	add    %ebx,%edi
      1a00:	72 32                	jb     0x1a34
      1a02:	66 81 fb 00 00 10 00 	cmp    $0x100000,%ebx
      1a09:	72 29                	jb     0x1a34
      1a0b:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      1a10:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      1a15:	75 28                	jne    0x1a3f
      1a17:	b8 00 08             	mov    $0x800,%ax
      1a1a:	5f                   	pop    %di
      1a1b:	5e                   	pop    %si
      1a1c:	59                   	pop    %cx
      1a1d:	5b                   	pop    %bx
      1a1e:	e8 71 f3             	call   0xd92
      1a21:	cd 31                	int    $0x31
      1a23:	e8 93 f3             	call   0xdb9
      1a26:	53                   	push   %bx
      1a27:	51                   	push   %cx
      1a28:	66 5b                	pop    %ebx
      1a2a:	72 0c                	jb     0x1a38
      1a2c:	e9 b3 00             	jmp    0x1ae2
      1a2f:	8f 06 f4 01          	pop    0x1f4
      1a33:	58                   	pop    %ax
      1a34:	66 59                	pop    %ecx
      1a36:	66 5b                	pop    %ebx
      1a38:	66 5f                	pop    %edi
      1a3a:	66 5e                	pop    %esi
      1a3c:	e9 70 f8             	jmp    0x12af
      1a3f:	66 f7 df             	neg    %edi
      1a42:	81 e7 00 f0          	and    $0xf000,%di
      1a46:	66 f7 df             	neg    %edi
      1a49:	81 e3 00 f0          	and    $0xf000,%bx
      1a4d:	66 2b fb             	sub    %ebx,%edi
      1a50:	66 c1 ef 0c          	shr    $0xc,%edi
      1a54:	66 8b f7             	mov    %edi,%esi
      1a57:	66 f7 df             	neg    %edi
      1a5a:	66 c1 ff 0a          	sar    $0xa,%edi
      1a5e:	66 f7 df             	neg    %edi
      1a61:	66 8b cf             	mov    %edi,%ecx
      1a64:	c1 e7 02             	shl    $0x2,%di
      1a67:	03 3e f4 01          	add    0x1f4,%di
      1a6b:	81 ff 00 10          	cmp    $0x1000,%di
      1a6f:	77 c3                	ja     0x1a34
      1a71:	87 3e f4 01          	xchg   %di,0x1f4
      1a75:	66 57                	push   %edi
      1a77:	66 81 c7 00 10 40 00 	add    $0x401000,%edi
      1a7e:	2e 8e 06 14 10       	mov    %cs:0x1014,%es
      1a83:	66 8b c1             	mov    %ecx,%eax
      1a86:	66 c1 e0 0c          	shl    $0xc,%eax
      1a8a:	e8 39 ff             	call   0x19c6
      1a8d:	72 a0                	jb     0x1a2f
      1a8f:	e8 97 10             	call   0x2b29
      1a92:	72 9b                	jb     0x1a2f
      1a94:	83 c7 04             	add    $0x4,%di
      1a97:	e2 f6                	loop   0x1a8f
      1a99:	66 5f                	pop    %edi
      1a9b:	66 57                	push   %edi
      1a9d:	66 c1 e7 0a          	shl    $0xa,%edi
      1aa1:	66 81 c7 00 00 40 00 	add    $0x400000,%edi
      1aa8:	66 8b ce             	mov    %esi,%ecx
      1aab:	66 f7 de             	neg    %esi
      1aae:	81 e6 00 fc          	and    $0xfc00,%si
      1ab2:	66 f7 de             	neg    %esi
      1ab5:	66 2b f1             	sub    %ecx,%esi
      1ab8:	66 8b c3             	mov    %ebx,%eax
      1abb:	b0 17                	mov    $0x17,%al
      1abd:	67 66 ab             	stos   %eax,%es:(%edi)
      1ac0:	66 05 00 10 00 00    	add    $0x1000,%eax
      1ac6:	67 e2 f4             	loopl  0x1abd
      1ac9:	8b ce                	mov    %si,%cx
      1acb:	66 33 c0             	xor    %eax,%eax
      1ace:	67 f3 66 ab          	rep stos %eax,%es:(%edi)
      1ad2:	66 5b                	pop    %ebx
      1ad4:	66 c1 e3 14          	shl    $0x14,%ebx
      1ad8:	66 59                	pop    %ecx
      1ada:	66 5f                	pop    %edi
      1adc:	81 e7 ff 0f          	and    $0xfff,%di
      1ae0:	0b df                	or     %di,%bx
      1ae2:	66 5f                	pop    %edi
      1ae4:	66 5e                	pop    %esi
      1ae6:	e9 29 fb             	jmp    0x1612
      1ae9:	66 58                	pop    %eax
      1aeb:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      1af0:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      1af5:	74 1b                	je     0x1b12
      1af7:	66 60                	pushal
      1af9:	2e ff 16 14 14       	call   *%cs:0x1414
      1afe:	e8 3d 0f             	call   0x2a3e
      1b01:	2e ff 16 16 14       	call   *%cs:0x1416
      1b06:	16                   	push   %ss
      1b07:	1f                   	pop    %ds
      1b08:	66 61                	popal
      1b0a:	66 ff 36 d8 04       	pushl  0x4d8
      1b0f:	e9 ef f3             	jmp    0xf01
      1b12:	e8 f2 1f             	call   0x3b07
      1b15:	66 50                	push   %eax
      1b17:	eb f6                	jmp    0x1b0f
      1b19:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      1b1e:	80 3e fd 04 01       	cmpb   $0x1,0x4fd
      1b23:	0f 84 88 f7          	je     0x12af
      1b27:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      1b2c:	74 27                	je     0x1b55
      1b2e:	66 39 0e d8 04       	cmp    %ecx,0x4d8
      1b33:	0f 82 78 f7          	jb     0x12af
      1b37:	66 29 0e d8 04       	sub    %ecx,0x4d8
      1b3c:	66 58                	pop    %eax
      1b3e:	66 ff 36 d0 04       	pushl  0x4d0
      1b43:	66 01 0e d0 04       	add    %ecx,0x4d0
      1b48:	66 8b 16 d4 04       	mov    0x4d4,%edx
      1b4d:	66 2b 16 d8 04       	sub    0x4d8,%edx
      1b52:	e9 ac f3             	jmp    0xf01
      1b55:	53                   	push   %bx
      1b56:	51                   	push   %cx
      1b57:	56                   	push   %si
      1b58:	57                   	push   %di
      1b59:	66 51                	push   %ecx
      1b5b:	59                   	pop    %cx
      1b5c:	5b                   	pop    %bx
      1b5d:	b8 01 05             	mov    $0x501,%ax
      1b60:	e8 2f f2             	call   0xd92
      1b63:	cd 31                	int    $0x31
      1b65:	e8 51 f2             	call   0xdb9
      1b68:	53                   	push   %bx
      1b69:	51                   	push   %cx
      1b6a:	66 58                	pop    %eax
      1b6c:	5f                   	pop    %di
      1b6d:	5e                   	pop    %si
      1b6e:	59                   	pop    %cx
      1b6f:	5b                   	pop    %bx
      1b70:	0f 82 3b f7          	jb     0x12af
      1b74:	66 2b 06 cc 04       	sub    0x4cc,%eax
      1b79:	66 5a                	pop    %edx
      1b7b:	66 50                	push   %eax
      1b7d:	66 8b 16 d4 04       	mov    0x4d4,%edx
      1b82:	e9 7c f3             	jmp    0xf01
      1b85:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      1b8a:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      1b8f:	74 3a                	je     0x1bcb
      1b91:	66 53                	push   %ebx
      1b93:	66 2b d9             	sub    %ecx,%ebx
      1b96:	72 2e                	jb     0x1bc6
      1b98:	81 e3 00 f0          	and    $0xf000,%bx
      1b9c:	66 53                	push   %ebx
      1b9e:	66 f7 db             	neg    %ebx
      1ba1:	66 03 1e d4 04       	add    0x4d4,%ebx
      1ba6:	66 29 1e d8 04       	sub    %ebx,0x4d8
      1bab:	72 12                	jb     0x1bbf
      1bad:	66 5a                	pop    %edx
      1baf:	66 5b                	pop    %ebx
      1bb1:	66 89 16 d4 04       	mov    %edx,0x4d4
      1bb6:	66 2b 16 d8 04       	sub    0x4d8,%edx
      1bbb:	66 4a                	dec    %edx
      1bbd:	eb 3e                	jmp    0x1bfd
      1bbf:	66 01 1e d8 04       	add    %ebx,0x4d8
      1bc4:	66 5b                	pop    %ebx
      1bc6:	66 5b                	pop    %ebx
      1bc8:	e9 e4 f6             	jmp    0x12af
      1bcb:	53                   	push   %bx
      1bcc:	51                   	push   %cx
      1bcd:	56                   	push   %si
      1bce:	57                   	push   %di
      1bcf:	66 51                	push   %ecx
      1bd1:	59                   	pop    %cx
      1bd2:	5b                   	pop    %bx
      1bd3:	b8 01 05             	mov    $0x501,%ax
      1bd6:	e8 b9 f1             	call   0xd92
      1bd9:	cd 31                	int    $0x31
      1bdb:	e8 db f1             	call   0xdb9
      1bde:	5f                   	pop    %di
      1bdf:	5e                   	pop    %si
      1be0:	72 1e                	jb     0x1c00
      1be2:	53                   	push   %bx
      1be3:	51                   	push   %cx
      1be4:	66 5a                	pop    %edx
      1be6:	66 2b 16 cc 04       	sub    0x4cc,%edx
      1beb:	59                   	pop    %cx
      1bec:	5b                   	pop    %bx
      1bed:	67 66 8d 1c 11       	lea    (%ecx,%edx,1),%ebx
      1bf2:	66 89 16 d4 04       	mov    %edx,0x4d4
      1bf7:	81 e2 00 f0          	and    $0xf000,%dx
      1bfb:	66 4a                	dec    %edx
      1bfd:	e9 01 f3             	jmp    0xf01
      1c00:	59                   	pop    %cx
      1c01:	5b                   	pop    %bx
      1c02:	eb c4                	jmp    0x1bc8
      1c04:	00 00                	add    %al,(%bx,%si)
      1c06:	50                   	push   %ax
      1c07:	8c c8                	mov    %cs,%ax
      1c09:	2e 3b 06 10 10       	cmp    %cs:0x1010,%ax
      1c0e:	75 49                	jne    0x1c59
      1c10:	66 0f b7 e4          	movzwl %sp,%esp
      1c14:	58                   	pop    %ax
      1c15:	67 87 04 24          	xchg   %ax,(%esp)
      1c19:	2e 2b 06 3a 2c       	sub    %cs:0x2c3a,%ax
      1c1e:	c1 e8 02             	shr    $0x2,%ax
      1c21:	67 66 8d 64 24 fa    	lea    -0x6(%esp),%esp
      1c27:	50                   	push   %ax
      1c28:	67 8b 44 24 08       	mov    0x8(%esp),%ax
      1c2d:	67 ff 74 24 12       	push   0x12(%esp)
      1c32:	67 81 24 24 ff bc    	andw   $0xbcff,(%esp)
      1c38:	9d                   	popf
      1c39:	e8 88 e6             	call   0x2c4
      1c3c:	9c                   	pushf
      1c3d:	67 87 04 24          	xchg   %ax,(%esp)
      1c41:	67 81 64 24 14 00 03 	andw   $0x300,0x14(%esp)
      1c48:	80 e4 bc             	and    $0xbc,%ah
      1c4b:	67 09 44 24 14       	or     %ax,0x14(%esp)
      1c50:	58                   	pop    %ax
      1c51:	67 66 8d 64 24 0a    	lea    0xa(%esp),%esp
      1c57:	66 cf                	iretl
      1c59:	8c d0                	mov    %ss,%ax
      1c5b:	3d 00 00             	cmp    $0x0,%ax
      1c5e:	0f 85 c4 00          	jne    0x1d26
      1c62:	36 66 ff 36 14 01    	pushl  %ss:0x114
      1c68:	66 16                	pushl  %ss
      1c6a:	66 54                	push   %esp
      1c6c:	66 0f b7 e4          	movzwl %sp,%esp
      1c70:	67 83 04 24 12       	addw   $0x12,(%esp)
      1c75:	66 9c                	pushfl
      1c77:	58                   	pop    %ax
      1c78:	67 ff 74 24 16       	push   0x16(%esp)
      1c7d:	67 81 24 24 ff bc    	andw   $0xbcff,(%esp)
      1c83:	67 81 0c 24 00 30    	orw    $0x3000,(%esp)
      1c89:	67 66 ff 74 24 16    	pushl  0x16(%esp)
      1c8f:	6a 00                	push   $0x0
      1c91:	67 ff 74 24 1a       	push   0x1a(%esp)
      1c96:	6a 01                	push   $0x1
      1c98:	67 8b 44 24 1c       	mov    0x1c(%esp),%ax
      1c9d:	2e 2b 06 3a 2c       	sub    %cs:0x2c3a,%ax
      1ca2:	c1 e8 02             	shr    $0x2,%ax
      1ca5:	50                   	push   %ax
      1ca6:	0f a8                	push   %gs
      1ca8:	0f a0                	push   %fs
      1caa:	06                   	push   %es
      1cab:	1e                   	push   %ds
      1cac:	67 8b 44 24 24       	mov    0x24(%esp),%ax
      1cb1:	66 50                	push   %eax
      1cb3:	66 54                	push   %esp
      1cb5:	33 c0                	xor    %ax,%ax
      1cb7:	2e ff 16 16 14       	call   *%cs:0x1416
      1cbc:	e8 cd 09             	call   0x268c
      1cbf:	66 58                	pop    %eax
      1cc1:	b8 01 00             	mov    $0x1,%ax
      1cc4:	2e ff 16 14 14       	call   *%cs:0x1414
      1cc9:	66 58                	pop    %eax
      1ccb:	67 8f 44 24 22       	pop    0x22(%esp)
      1cd0:	07                   	pop    %es
      1cd1:	0f a1                	pop    %fs
      1cd3:	0f a9                	pop    %gs
      1cd5:	83 c4 04             	add    $0x4,%sp
      1cd8:	67 66 8f 44 24 18    	popl   0x18(%esp)
      1cde:	67 8f 44 24 18       	pop    0x18(%esp)
      1ce3:	83 c4 02             	add    $0x2,%sp
      1ce6:	67 66 87 3c 24       	xchg   %edi,(%esp)
      1ceb:	67 81 64 24 18 00 02 	andw   $0x200,0x18(%esp)
      1cf2:	67 09 7c 24 18       	or     %di,0x18(%esp)
      1cf7:	06                   	push   %es
      1cf8:	56                   	push   %si
      1cf9:	67 66 c4 7c 24 08    	les    0x8(%esp),%edi
      1cff:	83 ef 1e             	sub    $0x1e,%di
      1d02:	16                   	push   %ss
      1d03:	1f                   	pop    %ds
      1d04:	8b f4                	mov    %sp,%si
      1d06:	fc                   	cld
      1d07:	51                   	push   %cx
      1d08:	b9 0f 00             	mov    $0xf,%cx
      1d0b:	f3 a5                	rep movsw %ds:(%si),%es:(%di)
      1d0d:	59                   	pop    %cx
      1d0e:	06                   	push   %es
      1d0f:	16                   	push   %ss
      1d10:	1f                   	pop    %ds
      1d11:	17                   	pop    %ss
      1d12:	67 66 8d 67 e2       	lea    -0x1e(%edi),%esp
      1d17:	5e                   	pop    %si
      1d18:	07                   	pop    %es
      1d19:	66 5f                	pop    %edi
      1d1b:	83 c4 08             	add    $0x8,%sp
      1d1e:	66 8f 06 14 01       	popl   0x114
      1d23:	66 1f                	popl   %ds
      1d25:	cf                   	iret
      1d26:	1e                   	push   %ds
      1d27:	66 53                	push   %ebx
      1d29:	bb 00 00             	mov    $0x0,%bx
      1d2c:	8e db                	mov    %bx,%ds
      1d2e:	81 2e 14 01 80 01    	subw   $0x180,0x114
      1d34:	66 8b 1e 14 01       	mov    0x114,%ebx
      1d39:	1e                   	push   %ds
      1d3a:	16                   	push   %ss
      1d3b:	1f                   	pop    %ds
      1d3c:	17                   	pop    %ss
      1d3d:	66 87 dc             	xchg   %ebx,%esp
      1d40:	66 ff 77 0c          	pushl  0xc(%bx)
      1d44:	66 ff 77 08          	pushl  0x8(%bx)
      1d48:	ff 77 06             	push   0x6(%bx)
      1d4b:	36 66 ff 36 14 01    	pushl  %ss:0x114
      1d51:	67 81 04 24 80 01    	addw   $0x180,(%esp)
      1d57:	66 1e                	pushl  %ds
      1d59:	66 53                	push   %ebx
      1d5b:	67 83 04 24 10       	addw   $0x10,(%esp)
      1d60:	66 c5 1f             	lds    (%bx),%ebx
      1d63:	e9 0f ff             	jmp    0x1c75
      1d66:	50                   	push   %ax
      1d67:	8c c8                	mov    %cs,%ax
      1d69:	2e 3b 06 10 10       	cmp    %cs:0x1010,%ax
      1d6e:	0f 85 e7 fe          	jne    0x1c59
      1d72:	58                   	pop    %ax
      1d73:	1e                   	push   %ds
      1d74:	66 56                	push   %esi
      1d76:	16                   	push   %ss
      1d77:	1f                   	pop    %ds
      1d78:	66 8b f4             	mov    %esp,%esi
      1d7b:	2e 8e 16 12 10       	mov    %cs:0x1012,%ss
      1d80:	36 66 8b 26 14 01    	mov    %ss:0x114,%esp
      1d86:	66 1e                	pushl  %ds
      1d88:	66 56                	push   %esi
      1d8a:	67 66 ff 76 10       	pushl  0x10(%esi)
      1d8f:	66 0e                	pushl  %cs
      1d91:	66 68 d9 2d 00 00    	pushl  $0x2dd9
      1d97:	50                   	push   %ax
      1d98:	67 8b 46 06          	mov    0x6(%esi),%ax
      1d9c:	67 66 c5 36          	lds    (%esi),%esi
      1da0:	e9 76 fe             	jmp    0x1c19
      1da3:	67 66 0f b2 24 24    	lss    (%esp),%esp
      1da9:	9c                   	pushf
      1daa:	67 8f 44 24 10       	pop    0x10(%esp)
      1daf:	66 83 c4 08          	add    $0x8,%esp
      1db3:	66 cf                	iretl
      1db5:	66 0f b7 e4          	movzwl %sp,%esp
      1db9:	66 1e                	pushl  %ds
      1dbb:	9c                   	pushf
      1dbc:	3c 09                	cmp    $0x9,%al
      1dbe:	0f 84 19 00          	je     0x1ddb
      1dc2:	9d                   	popf
      1dc3:	66 6a 00             	pushl  $0x0
      1dc6:	36 66 ff 36 ec 01    	pushl  %ss:0x1ec
      1dcc:	6a 33                	push   $0x33
      1dce:	e8 f3 e4             	call   0x2c4
      1dd1:	67 66 8d 64 24 0a    	lea    0xa(%esp),%esp
      1dd7:	66 1f                	popl   %ds
      1dd9:	66 cf                	iretl
      1ddb:	66 56                	push   %esi
      1ddd:	66 57                	push   %edi
      1ddf:	66 51                	push   %ecx
      1de1:	06                   	push   %es
      1de2:	1f                   	pop    %ds
      1de3:	66 8b f2             	mov    %edx,%esi
      1de6:	36 66 8b 3e f0 01    	mov    %ss:0x1f0,%edi
      1dec:	2e 8e 06 14 10       	mov    %cs:0x1014,%es
      1df1:	66 33 d2             	xor    %edx,%edx
      1df4:	66 b9 10 00 00 00    	mov    $0x10,%ecx
      1dfa:	fc                   	cld
      1dfb:	67 f3 66 a5          	rep movsl %ds:(%esi),%es:(%edi)
      1dff:	1e                   	push   %ds
      1e00:	07                   	pop    %es
      1e01:	66 59                	pop    %ecx
      1e03:	66 5f                	pop    %edi
      1e05:	66 5e                	pop    %esi
      1e07:	eb b9                	jmp    0x1dc2
      1e09:	66 1e                	pushl  %ds
      1e0b:	16                   	push   %ss
      1e0c:	1f                   	pop    %ds
      1e0d:	66 56                	push   %esi
      1e0f:	66 8b f4             	mov    %esp,%esi
      1e12:	2e 8e 16 12 10       	mov    %cs:0x1012,%ss
      1e17:	36 66 8b 26 14 01    	mov    %ss:0x114,%esp
      1e1d:	66 1e                	pushl  %ds
      1e1f:	66 56                	push   %esi
      1e21:	67 66 ff 76 10       	pushl  0x10(%esi)
      1e26:	66 0e                	pushl  %cs
      1e28:	66 68 6a 2e 00 00    	pushl  $0x2e6a
      1e2e:	67 66 c5 36          	lds    (%esi),%esi
      1e32:	eb 85                	jmp    0x1db9
      1e34:	67 66 0f b2 24 24    	lss    (%esp),%esp
      1e3a:	67 66 8d 64 24 08    	lea    0x8(%esp),%esp
      1e40:	9c                   	pushf
      1e41:	67 8f 44 24 08       	pop    0x8(%esp)
      1e46:	66 cf                	iretl
      1e48:	66 0f b7 c3          	movzwl %bx,%eax
      1e4c:	24 f8                	and    $0xf8,%al
      1e4e:	74 06                	je     0x1e56
      1e50:	05 10 00             	add    $0x10,%ax
      1e53:	3d d0 00             	cmp    $0xd0,%ax
      1e56:	f5                   	cmc
      1e57:	c3                   	ret
      1e58:	00 b8 24 35          	add    %bh,0x3524(%bx,%si)
      1e5c:	cd 21                	int    $0x21
      1e5e:	89 1e 00 02          	mov    %bx,0x200
      1e62:	8c 06 02 02          	mov    %es,0x202
      1e66:	1e                   	push   %ds
      1e67:	0e                   	push   %cs
      1e68:	1f                   	pop    %ds
      1e69:	b8 23 25             	mov    $0x2523,%ax
      1e6c:	ba b0 2e             	mov    $0x2eb0,%dx
      1e6f:	cd 21                	int    $0x21
      1e71:	fe c0                	inc    %al
      1e73:	ba d8 2e             	mov    $0x2ed8,%dx
      1e76:	cd 21                	int    $0x21
      1e78:	1f                   	pop    %ds
      1e79:	c3                   	ret
      1e7a:	2e c6 06 8e 2e 23    	movb   $0x23,%cs:0x2e8e
      1e80:	e9 0c e9             	jmp    0x78f
      1e83:	57                   	push   %di
      1e84:	50                   	push   %ax
      1e85:	51                   	push   %cx
      1e86:	06                   	push   %es
      1e87:	b8 86 17             	mov    $0x1786,%ax
      1e8a:	2e 8e 06 12 10       	mov    %cs:0x1012,%es
      1e8f:	b9 76 1e             	mov    $0x1e76,%cx
      1e92:	bf be 1d             	mov    $0x1dbe,%di
      1e95:	2b cf                	sub    %di,%cx
      1e97:	d1 e9                	shr    $1,%cx
      1e99:	f3 ab                	rep stos %ax,%es:(%di)
      1e9b:	07                   	pop    %es
      1e9c:	59                   	pop    %cx
      1e9d:	58                   	pop    %ax
      1e9e:	5f                   	pop    %di
      1e9f:	66 cf                	iretl
      1ea1:	cf                   	iret
      1ea2:	1e                   	push   %ds
      1ea3:	68 00 00             	push   $0x0
      1ea6:	1f                   	pop    %ds
      1ea7:	9c                   	pushf
      1ea8:	ff 1e 00 02          	lcall  *0x200
      1eac:	1f                   	pop    %ds
      1ead:	3c 02                	cmp    $0x2,%al
      1eaf:	75 f0                	jne    0x1ea1
      1eb1:	66 60                	pushal
      1eb3:	1e                   	push   %ds
      1eb4:	06                   	push   %es
      1eb5:	2e c6 06 8e 2e 24    	movb   $0x24,%cs:0x2e8e
      1ebb:	e9 d1 e8             	jmp    0x78f
      1ebe:	cd 08                	int    $0x8
      1ec0:	cf                   	iret
      1ec1:	cd 09                	int    $0x9
      1ec3:	cf                   	iret
      1ec4:	cd 0a                	int    $0xa
      1ec6:	cf                   	iret
      1ec7:	cd 0b                	int    $0xb
      1ec9:	cf                   	iret
      1eca:	cd 0c                	int    $0xc
      1ecc:	cf                   	iret
      1ecd:	cd 0d                	int    $0xd
      1ecf:	cf                   	iret
      1ed0:	cd 0e                	int    $0xe
      1ed2:	cf                   	iret
      1ed3:	cd 0f                	int    $0xf
      1ed5:	cf                   	iret
      1ed6:	cd 70                	int    $0x70
      1ed8:	cf                   	iret
      1ed9:	cd 71                	int    $0x71
      1edb:	cf                   	iret
      1edc:	cd 72                	int    $0x72
      1ede:	cf                   	iret
      1edf:	cd 73                	int    $0x73
      1ee1:	cf                   	iret
      1ee2:	cd 74                	int    $0x74
      1ee4:	cf                   	iret
      1ee5:	cd 75                	int    $0x75
      1ee7:	cf                   	iret
      1ee8:	cd 76                	int    $0x76
      1eea:	cf                   	iret
      1eeb:	cd 77                	int    $0x77
      1eed:	cf                   	iret
      1eee:	6a 08                	push   $0x8
      1ef0:	eb 3a                	jmp    0x1f2c
      1ef2:	6a 09                	push   $0x9
      1ef4:	eb 36                	jmp    0x1f2c
      1ef6:	6a 0a                	push   $0xa
      1ef8:	eb 32                	jmp    0x1f2c
      1efa:	6a 0b                	push   $0xb
      1efc:	eb 2e                	jmp    0x1f2c
      1efe:	6a 0c                	push   $0xc
      1f00:	eb 2a                	jmp    0x1f2c
      1f02:	6a 0d                	push   $0xd
      1f04:	eb 26                	jmp    0x1f2c
      1f06:	6a 0e                	push   $0xe
      1f08:	eb 22                	jmp    0x1f2c
      1f0a:	6a 0f                	push   $0xf
      1f0c:	eb 1e                	jmp    0x1f2c
      1f0e:	6a 70                	push   $0x70
      1f10:	eb 1a                	jmp    0x1f2c
      1f12:	6a 71                	push   $0x71
      1f14:	eb 16                	jmp    0x1f2c
      1f16:	6a 72                	push   $0x72
      1f18:	eb 12                	jmp    0x1f2c
      1f1a:	6a 73                	push   $0x73
      1f1c:	eb 0e                	jmp    0x1f2c
      1f1e:	6a 74                	push   $0x74
      1f20:	eb 0a                	jmp    0x1f2c
      1f22:	6a 75                	push   $0x75
      1f24:	eb 06                	jmp    0x1f2c
      1f26:	6a 76                	push   $0x76
      1f28:	eb 02                	jmp    0x1f2c
      1f2a:	6a 77                	push   $0x77
      1f2c:	66 0f b7 e4          	movzwl %sp,%esp
      1f30:	1e                   	push   %ds
      1f31:	66 50                	push   %eax
      1f33:	06                   	push   %es
      1f34:	0f a0                	push   %fs
      1f36:	0f a8                	push   %gs
      1f38:	2e ff 16 14 14       	call   *%cs:0x1414
      1f3d:	9c                   	pushf
      1f3e:	67 66 0f b6 44 24 0e 	movzbl 0xe(%esp),%eax
      1f45:	36 8e 1e 04 02       	mov    %ss:0x204,%ds
      1f4a:	67 f6 04 85 03 00 00 	testb  $0x1,0x3(,%eax,4)
      1f51:	00 01 
      1f53:	75 0e                	jne    0x1f63
      1f55:	24 1f                	and    $0x1f,%al
      1f57:	0e                   	push   %cs
      1f58:	68 a7 2f             	push   $0x2fa7
      1f5b:	6b c0 03             	imul   $0x3,%ax,%ax
      1f5e:	05 dc 2e             	add    $0x2edc,%ax
      1f61:	ff e0                	jmp    *%ax
      1f63:	36 66 8e 1e 1c 02    	data32 mov %ss:0x21c,%ds
      1f69:	67 ff 1c 85 00 00 00 	lcall  *0x0(,%eax,4)
      1f70:	00 
      1f71:	2e ff 16 16 14       	call   *%cs:0x1416
      1f76:	0f a9                	pop    %gs
      1f78:	0f a1                	pop    %fs
      1f7a:	07                   	pop    %es
      1f7b:	66 58                	pop    %eax
      1f7d:	1f                   	pop    %ds
      1f7e:	83 c4 02             	add    $0x2,%sp
      1f81:	66 cf                	iretl
      1f83:	67 6b 7c 24 06 06    	imul   $0x6,0x6(%esp),%di
      1f89:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      1f8e:	ff b5 10 06          	push   0x610(%di)
      1f92:	66 ff b5 0c 06       	pushl  0x60c(%di)
      1f97:	67 8e 5c 24 0a       	mov    0xa(%esp),%ds
      1f9c:	67 8b 7c 24 06       	mov    0x6(%esp),%di
      1fa1:	67 66 8f 44 24 02    	popl   0x2(%esp)
      1fa7:	67 8f 44 24 04       	pop    0x4(%esp)
      1fac:	66 cb                	lretl
      1fae:	1e                   	push   %ds
      1faf:	66 57                	push   %edi
      1fb1:	67 8b 7c 24 18       	mov    0x18(%esp),%di
      1fb6:	2e 3b 3e 10 10       	cmp    %cs:0x1010,%di
      1fbb:	74 07                	je     0x1fc4
      1fbd:	2e 3b 3e 16 10       	cmp    %cs:0x1016,%di
      1fc2:	75 bf                	jne    0x1f83
      1fc4:	66 56                	push   %esi
      1fc6:	66 51                	push   %ecx
      1fc8:	06                   	push   %es
      1fc9:	2e 8e 06 12 10       	mov    %cs:0x1012,%es
      1fce:	26 66 8b 3e 14 01    	mov    %es:0x114,%edi
      1fd4:	83 ef 18             	sub    $0x18,%di
      1fd7:	67 66 8d 74 24 1a    	lea    0x1a(%esp),%esi
      1fdd:	16                   	push   %ss
      1fde:	1f                   	pop    %ds
      1fdf:	66 b9 06 00 00 00    	mov    $0x6,%ecx
      1fe5:	fc                   	cld
      1fe6:	67 f3 66 a5          	rep movsl %ds:(%esi),%es:(%edi)
      1fea:	83 ef 1a             	sub    $0x1a,%di
      1fed:	67 8b 74 24 10       	mov    0x10(%esp),%si
      1ff2:	26 89 35             	mov    %si,%es:(%di)
      1ff5:	67 8c 44 24 2e       	mov    %es,0x2e(%esp)
      1ffa:	67 66 89 7c 24 2a    	mov    %edi,0x2a(%esp)
      2000:	67 8c 4c 24 22       	mov    %cs,0x22(%esp)
      2005:	bf 5a 30             	mov    $0x305a,%di
      2008:	67 66 89 7c 24 1e    	mov    %edi,0x1e(%esp)
      200e:	67 80 64 24 27 fe    	andb   $0xfe,0x27(%esp)
      2014:	07                   	pop    %es
      2015:	66 59                	pop    %ecx
      2017:	66 5e                	pop    %esi
      2019:	66 5f                	pop    %edi
      201b:	1f                   	pop    %ds
      201c:	67 66 8d 64 24 02    	lea    0x2(%esp),%esp
      2022:	66 cb                	lretl
      2024:	0f a0                	push   %fs
      2026:	06                   	push   %es
      2027:	1e                   	push   %ds
      2028:	66 50                	push   %eax
      202a:	67 66 ff 74 24 0c    	pushl  0xc(%esp)
      2030:	6a 02                	push   $0x2
      2032:	67 ff 74 24 10       	push   0x10(%esp)
      2037:	67 66 8f 44 24 10    	popl   0x10(%esp)
      203d:	67 8c 6c 24 0e       	mov    %gs,0xe(%esp)
      2042:	67 66 8d 44 24 04    	lea    0x4(%esp),%eax
      2048:	66 50                	push   %eax
      204a:	16                   	push   %ss
      204b:	1f                   	pop    %ds
      204c:	eb 27                	jmp    0x2075
      204e:	1e                   	push   %ds
      204f:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      2054:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      2059:	1f                   	pop    %ds
      205a:	0f 84 50 ff          	je     0x1fae
      205e:	66 0f b7 e4          	movzwl %sp,%esp
      2062:	6a 00                	push   $0x0
      2064:	67 ff 74 24 02       	push   0x2(%esp)
      2069:	67 c7 44 24 04 00 00 	movw   $0x0,0x4(%esp)
      2070:	6a 02                	push   $0x2
      2072:	e8 cf 01             	call   0x2244
      2075:	53                   	push   %bx
      2076:	67 8b 5c 24 02       	mov    0x2(%esp),%bx
      207b:	8b 5f 0c             	mov    0xc(%bx),%bx
      207e:	0f a3 1e fc 01       	bt     %bx,0x1fc
      2083:	5b                   	pop    %bx
      2084:	0f 82 51 02          	jb     0x22d9
      2088:	eb 79                	jmp    0x2103
      208a:	90                   	nop
      208b:	6a 00                	push   $0x0
      208d:	eb bf                	jmp    0x204e
      208f:	6a 01                	push   $0x1
      2091:	eb bb                	jmp    0x204e
      2093:	6a 02                	push   $0x2
      2095:	eb b7                	jmp    0x204e
      2097:	6a 03                	push   $0x3
      2099:	eb b3                	jmp    0x204e
      209b:	6a 04                	push   $0x4
      209d:	eb af                	jmp    0x204e
      209f:	6a 05                	push   $0x5
      20a1:	eb ab                	jmp    0x204e
      20a3:	6a 06                	push   $0x6
      20a5:	eb a7                	jmp    0x204e
      20a7:	6a 07                	push   $0x7
      20a9:	eb a3                	jmp    0x204e
      20ab:	6a 08                	push   $0x8
      20ad:	eb 1a                	jmp    0x20c9
      20af:	6a 09                	push   $0x9
      20b1:	eb 16                	jmp    0x20c9
      20b3:	6a 0a                	push   $0xa
      20b5:	eb 12                	jmp    0x20c9
      20b7:	6a 0b                	push   $0xb
      20b9:	eb 0e                	jmp    0x20c9
      20bb:	6a 0c                	push   $0xc
      20bd:	eb 0a                	jmp    0x20c9
      20bf:	6a 0d                	push   $0xd
      20c1:	eb 06                	jmp    0x20c9
      20c3:	6a 0e                	push   $0xe
      20c5:	eb 02                	jmp    0x20c9
      20c7:	6a 0f                	push   $0xf
      20c9:	66 0f b7 e4          	movzwl %sp,%esp
      20cd:	50                   	push   %ax
      20ce:	53                   	push   %bx
      20cf:	67 8b 5c 24 04       	mov    0x4(%esp),%bx
      20d4:	67 8b 44 24 0e       	mov    0xe(%esp),%ax
      20d9:	2e 3b 06 16 10       	cmp    %cs:0x1016,%ax
      20de:	74 0a                	je     0x20ea
      20e0:	2e 3b 06 10 10       	cmp    %cs:0x1010,%ax
      20e5:	74 03                	je     0x20ea
      20e7:	80 c3 08             	add    $0x8,%bl
      20ea:	36 0f a3 1e fc 01    	bt     %bx,%ss:0x1fc
      20f0:	0f 82 cb 01          	jb     0x22bf
      20f4:	0f ba e3 04          	bt     $0x4,%bx
      20f8:	5b                   	pop    %bx
      20f9:	58                   	pop    %ax
      20fa:	0f 82 32 fe          	jb     0x1f30
      20fe:	6a 02                	push   $0x2
      2100:	e8 41 01             	call   0x2244
      2103:	67 8a 44 24 14       	mov    0x14(%esp),%al
      2108:	68 4e 31             	push   $0x314e
      210b:	3c 0e                	cmp    $0xe,%al
      210d:	0f 84 8a 0b          	je     0x2c9b
      2111:	3c 0d                	cmp    $0xd,%al
      2113:	0f 84 42 02          	je     0x2359
      2117:	58                   	pop    %ax
      2118:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      211d:	74 03                	je     0x2122
      211f:	0f 20 d0             	mov    %cr2,%eax
      2122:	66 50                	push   %eax
      2124:	32 c0                	xor    %al,%al
      2126:	2e ff 16 14 14       	call   *%cs:0x1414
      212b:	66 60                	pushal
      212d:	8b ec                	mov    %sp,%bp
      212f:	1e                   	push   %ds
      2130:	36 66 8e 1e 1c 02    	data32 mov %ss:0x21c,%ds
      2136:	33 f6                	xor    %si,%si
      2138:	8b fe                	mov    %si,%di
      213a:	8e c7                	mov    %di,%es
      213c:	b9 00 01             	mov    $0x100,%cx
      213f:	f3 66 a5             	rep movsl %ds:(%si),%es:(%di)
      2142:	1f                   	pop    %ds
      2143:	67 8b 5c 24 24       	mov    0x24(%esp),%bx
      2148:	8a 47 0c             	mov    0xc(%bx),%al
      214b:	3c 0e                	cmp    $0xe,%al
      214d:	75 29                	jne    0x2178
      214f:	8b 16 40 02          	mov    0x240,%dx
      2153:	e8 db 01             	call   0x2331
      2156:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      215b:	74 50                	je     0x21ad
      215d:	66 8b 46 20          	mov    0x20(%bp),%eax
      2161:	66 50                	push   %eax
      2163:	e8 8a 0f             	call   0x30f0
      2166:	ba 24 03             	mov    $0x324,%dx
      2169:	e8 c5 01             	call   0x2331
      216c:	66 58                	pop    %eax
      216e:	66 2b 06 cc 04       	sub    0x4cc,%eax
      2173:	e8 7a 0f             	call   0x30f0
      2176:	eb 35                	jmp    0x21ad
      2178:	3c 0c                	cmp    $0xc,%al
      217a:	75 08                	jne    0x2184
      217c:	ba 1e 04             	mov    $0x41e,%dx
      217f:	e8 af 01             	call   0x2331
      2182:	eb 29                	jmp    0x21ad
      2184:	3c 0d                	cmp    $0xd,%al
      2186:	75 19                	jne    0x21a1
      2188:	ba 3b 04             	mov    $0x43b,%dx
      218b:	e8 a3 01             	call   0x2331
      218e:	83 7f fc 00          	cmpw   $0x0,-0x4(%bx)
      2192:	75 05                	jne    0x2199
      2194:	ba 65 04             	mov    $0x465,%dx
      2197:	eb 03                	jmp    0x219c
      2199:	ba 84 04             	mov    $0x484,%dx
      219c:	e8 92 01             	call   0x2331
      219f:	eb 0c                	jmp    0x21ad
      21a1:	ba a3 04             	mov    $0x4a3,%dx
      21a4:	e8 8a 01             	call   0x2331
      21a7:	8a 47 0c             	mov    0xc(%bx),%al
      21aa:	e8 59 0f             	call   0x3106
      21ad:	66 33 ff             	xor    %edi,%edi
      21b0:	8a 47 0c             	mov    0xc(%bx),%al
      21b3:	24 f8                	and    $0xf8,%al
      21b5:	34 08                	xor    $0x8,%al
      21b7:	75 0c                	jne    0x21c5
      21b9:	ba 46 03             	mov    $0x346,%dx
      21bc:	e8 72 01             	call   0x2331
      21bf:	8b 47 fc             	mov    -0x4(%bx),%ax
      21c2:	e8 1e 0f             	call   0x30e3
      21c5:	be 20 02             	mov    $0x220,%si
      21c8:	66 8b 07             	mov    (%bx),%eax
      21cb:	f9                   	stc
      21cc:	e8 45 01             	call   0x2314
      21cf:	66 8b 46 04          	mov    0x4(%bp),%eax
      21d3:	e8 3e 01             	call   0x2314
      21d6:	f8                   	clc
      21d7:	8b 47 18             	mov    0x18(%bx),%ax
      21da:	e8 37 01             	call   0x2314
      21dd:	8b 47 04             	mov    0x4(%bx),%ax
      21e0:	e8 31 01             	call   0x2314
      21e3:	f9                   	stc
      21e4:	66 8b 46 10          	mov    0x10(%bp),%eax
      21e8:	e8 29 01             	call   0x2314
      21eb:	66 8b 46 00          	mov    0x0(%bp),%eax
      21ef:	e8 22 01             	call   0x2314
      21f2:	66 8b 47 10          	mov    0x10(%bx),%eax
      21f6:	e8 1b 01             	call   0x2314
      21f9:	f8                   	clc
      21fa:	8b 47 06             	mov    0x6(%bx),%ax
      21fd:	e8 14 01             	call   0x2314
      2200:	f9                   	stc
      2201:	66 8b 46 18          	mov    0x18(%bp),%eax
      2205:	e8 0c 01             	call   0x2314
      2208:	66 8b 46 08          	mov    0x8(%bp),%eax
      220c:	e8 05 01             	call   0x2314
      220f:	f8                   	clc
      2210:	8b 47 14             	mov    0x14(%bx),%ax
      2213:	e8 fe 00             	call   0x2314
      2216:	8b 47 08             	mov    0x8(%bx),%ax
      2219:	e8 f8 00             	call   0x2314
      221c:	f9                   	stc
      221d:	66 8b 46 14          	mov    0x14(%bp),%eax
      2221:	e8 f0 00             	call   0x2314
      2224:	66 8b 47 1c          	mov    0x1c(%bx),%eax
      2228:	e8 e9 00             	call   0x2314
      222b:	8b 47 20             	mov    0x20(%bx),%ax
      222e:	f8                   	clc
      222f:	e8 e2 00             	call   0x2314
      2232:	8b 47 0a             	mov    0xa(%bx),%ax
      2235:	e8 dc 00             	call   0x2314
      2238:	b0 01                	mov    $0x1,%al
      223a:	2e ff 16 16 14       	call   *%cs:0x1416
      223f:	6a 01                	push   $0x1
      2241:	e9 0e e5             	jmp    0x752
      2244:	57                   	push   %di
      2245:	8c cf                	mov    %cs,%di
      2247:	67 3b 7c 24 10       	cmp    0x10(%esp),%di
      224c:	8b fc                	mov    %sp,%di
      224e:	75 3f                	jne    0x228f
      2250:	0f a8                	push   %gs
      2252:	0f a0                	push   %fs
      2254:	06                   	push   %es
      2255:	1e                   	push   %ds
      2256:	66 50                	push   %eax
      2258:	16                   	push   %ss
      2259:	1f                   	pop    %ds
      225a:	66 ff 75 08          	pushl  0x8(%di)
      225e:	67 66 8d 44 24 04    	lea    0x4(%esp),%eax
      2264:	66 50                	push   %eax
      2266:	66 ff 35             	pushl  (%di)
      2269:	16                   	push   %ss
      226a:	07                   	pop    %es
      226b:	56                   	push   %si
      226c:	51                   	push   %cx
      226d:	66 8b 45 04          	mov    0x4(%di),%eax
      2271:	66 c1 c8 10          	ror    $0x10,%eax
      2275:	66 ab                	stos   %eax,%es:(%di)
      2277:	8d 75 08             	lea    0x8(%di),%si
      227a:	b9 03 00             	mov    $0x3,%cx
      227d:	fc                   	cld
      227e:	f3 66 a5             	rep movsl %ds:(%si),%es:(%di)
      2281:	66 0f b7 c6          	movzwl %si,%eax
      2285:	66 ab                	stos   %eax,%es:(%di)
      2287:	8c d0                	mov    %ss,%ax
      2289:	66 ab                	stos   %eax,%es:(%di)
      228b:	59                   	pop    %cx
      228c:	5e                   	pop    %si
      228d:	5f                   	pop    %di
      228e:	c3                   	ret
      228f:	66 50                	push   %eax
      2291:	36 66 ff 75 08       	pushl  %ss:0x8(%di)
      2296:	67 66 8d 44 24 04    	lea    0x4(%esp),%eax
      229c:	66 50                	push   %eax
      229e:	36 66 ff 35          	pushl  %ss:(%di)
      22a2:	06                   	push   %es
      22a3:	1e                   	push   %ds
      22a4:	16                   	push   %ss
      22a5:	1f                   	pop    %ds
      22a6:	66 8f 05             	popl   (%di)
      22a9:	66 8b 45 04          	mov    0x4(%di),%eax
      22ad:	66 c1 c8 10          	ror    $0x10,%eax
      22b1:	66 89 45 08          	mov    %eax,0x8(%di)
      22b5:	0f a8                	push   %gs
      22b7:	0f a0                	push   %fs
      22b9:	66 8f 45 04          	popl   0x4(%di)
      22bd:	5f                   	pop    %di
      22be:	c3                   	ret
      22bf:	b8 02 00             	mov    $0x2,%ax
      22c2:	0f ba e3 04          	bt     $0x4,%bx
      22c6:	5b                   	pop    %bx
      22c7:	0f 83 07 00          	jae    0x22d2
      22cb:	67 66 ff 34 24       	pushl  (%esp)
      22d0:	33 c0                	xor    %ax,%ax
      22d2:	67 87 04 24          	xchg   %ax,(%esp)
      22d6:	e8 6b ff             	call   0x2244
      22d9:	67 87 1c 24          	xchg   %bx,(%esp)
      22dd:	66 8b 47 0c          	mov    0xc(%bx),%eax
      22e1:	67 87 1c 24          	xchg   %bx,(%esp)
      22e5:	68 33 33             	push   $0x3333
      22e8:	66 3d 0e 00 02 00    	cmp    $0x2000e,%eax
      22ee:	0f 84 a9 09          	je     0x2c9b
      22f2:	66 3d 0d 00 02 00    	cmp    $0x2000d,%eax
      22f8:	0f 84 5d 00          	je     0x2359
      22fc:	58                   	pop    %ax
      22fd:	e8 8c 03             	call   0x268c
      2300:	36 80 3e fc 04 c0    	cmpb   $0xc0,%ss:0x4fc
      2306:	0f 85 58 02          	jne    0x2562
      230a:	36 81 2e 14 01 80 01 	subw   $0x180,%ss:0x114
      2311:	e9 31 03             	jmp    0x2645
      2314:	9c                   	pushf
      2315:	66 50                	push   %eax
      2317:	9c                   	pushf
      2318:	8b 14                	mov    (%si),%dx
      231a:	e8 14 00             	call   0x2331
      231d:	83 c6 02             	add    $0x2,%si
      2320:	9d                   	popf
      2321:	66 58                	pop    %eax
      2323:	0f 83 05 00          	jae    0x232c
      2327:	e8 c6 0d             	call   0x30f0
      232a:	9d                   	popf
      232b:	c3                   	ret
      232c:	e8 b4 0d             	call   0x30e3
      232f:	9d                   	popf
      2330:	c3                   	ret
      2331:	b4 09                	mov    $0x9,%ah
      2333:	cd 21                	int    $0x21
      2335:	c3                   	ret
      2336:	66 03 06 0c 02       	add    0x20c,%eax
      233b:	66 83 f8 1e          	cmp    $0x1e,%eax
      233f:	76 01                	jbe    0x2342
      2341:	c3                   	ret
      2342:	67 66 8b 44 24 02    	mov    0x2(%esp),%eax
      2348:	67 66 c4 40 1c       	les    0x1c(%eax),%eax
      234d:	26 67 66 0f b7 40 10 	movzwl %es:0x10(%eax),%eax
      2354:	66 50                	push   %eax
      2356:	e9 bf fd             	jmp    0x2118
      2359:	2e 8e 06 18 10       	mov    %cs:0x1018,%es
      235e:	67 66 8b 44 24 02    	mov    0x2(%esp),%eax
      2364:	67 66 8b 40 10       	mov    0x10(%eax),%eax
      2369:	66 2b 06 08 02       	sub    0x208,%eax
      236e:	72 c6                	jb     0x2336
      2370:	66 3d fe 01 00 00    	cmp    $0x1fe,%eax
      2376:	77 be                	ja     0x2336
      2378:	53                   	push   %bx
      2379:	c1 e0 02             	shl    $0x2,%ax
      237c:	50                   	push   %ax
      237d:	66 03 06 10 02       	add    0x210,%eax
      2382:	26 67 66 0f b7 40 06 	movzwl %es:0x6(%eax),%eax
      2389:	50                   	push   %ax
      238a:	0b c0                	or     %ax,%ax
      238c:	0f 84 b9 00          	je     0x2449
      2390:	67 66 8b 00          	mov    (%eax),%eax
      2394:	67 8b 5c 24 08       	mov    0x8(%esp),%bx
      2399:	66 3b 07             	cmp    (%bx),%eax
      239c:	0f 85 a9 00          	jne    0x2449
      23a0:	67 8b 1c 24          	mov    (%esp),%bx
      23a4:	66 ff 77 08          	pushl  0x8(%bx)
      23a8:	66 ff 77 04          	pushl  0x4(%bx)
      23ac:	67 8b 5c 24 0a       	mov    0xa(%esp),%bx
      23b1:	c1 eb 03             	shr    $0x3,%bx
      23b4:	53                   	push   %bx
      23b5:	67 8b 5c 24 12       	mov    0x12(%esp),%bx
      23ba:	67 8b 5c 24 0e       	mov    0xe(%esp),%bx
      23bf:	66 53                	push   %ebx
      23c1:	67 8b 5c 24 16       	mov    0x16(%esp),%bx
      23c6:	1e                   	push   %ds
      23c7:	66 c5 5f 1c          	lds    0x1c(%bx),%ebx
      23cb:	67 8b 5b 0c          	mov    0xc(%ebx),%bx
      23cf:	1f                   	pop    %ds
      23d0:	80 e7 fc             	and    $0xfc,%bh
      23d3:	80 cf 30             	or     $0x30,%bh
      23d6:	53                   	push   %bx
      23d7:	9d                   	popf
      23d8:	66 5b                	pop    %ebx
      23da:	e8 e7 de             	call   0x2c4
      23dd:	67 66 8d 64 24 02    	lea    0x2(%esp),%esp
      23e3:	66 50                	push   %eax
      23e5:	67 66 8b 44 24 14    	mov    0x14(%esp),%eax
      23eb:	67 66 8f 00          	popl   (%eax)
      23ef:	9c                   	pushf
      23f0:	67 8f 40 18          	pop    0x18(%eax)
      23f4:	67 66 0f b7 44 24 08 	movzwl 0x8(%esp),%eax
      23fb:	67 66 8f 40 04       	popl   0x4(%eax)
      2400:	36 67 66 8f 40 08    	popl   %ss:0x8(%eax)
      2406:	66 58                	pop    %eax
      2408:	66 58                	pop    %eax
      240a:	66 5c                	pop    %esp
      240c:	66 58                	pop    %eax
      240e:	1f                   	pop    %ds
      240f:	07                   	pop    %es
      2410:	0f a1                	pop    %fs
      2412:	0f a9                	pop    %gs
      2414:	67 66 8d 64 24 04    	lea    0x4(%esp),%esp
      241a:	36 80 3e fc 04 c0    	cmpb   $0xc0,%ss:0x4fc
      2420:	74 0b                	je     0x242d
      2422:	67 66 c7 04 24 0b bf 	movl   $0xbf0b,(%esp)
      2429:	00 00 
      242b:	66 cf                	iretl
      242d:	67 66 8d 64 24 08    	lea    0x8(%esp),%esp
      2433:	66 9d                	popfl
      2435:	67 66 0f b2 24 24    	lss    (%esp),%esp
      243b:	67 66 8d 64 24 04    	lea    0x4(%esp),%esp
      2441:	9c                   	pushf
      2442:	67 8f 44 24 08       	pop    0x8(%esp)
      2447:	66 cf                	iretl
      2449:	58                   	pop    %ax
      244a:	58                   	pop    %ax
      244b:	66 0f b7 c0          	movzwl %ax,%eax
      244f:	66 03 06 14 02       	add    0x214,%eax
      2454:	67 8b 5c 24 04       	mov    0x4(%esp),%bx
      2459:	26 67 ff 70 04       	push   %es:0x4(%eax)
      245e:	26 67 66 ff 30       	pushl  %es:(%eax)
      2463:	8e 6f 0a             	mov    0xa(%bx),%gs
      2466:	8e 67 08             	mov    0x8(%bx),%fs
      2469:	66 8f 47 08          	popl   0x8(%bx)
      246d:	8f 47 0c             	pop    0xc(%bx)
      2470:	5b                   	pop    %bx
      2471:	58                   	pop    %ax
      2472:	66 5c                	pop    %esp
      2474:	67 66 c5 44 24 1c    	lds    0x1c(%esp),%eax
      247a:	67 66 ff 70 04       	pushl  0x4(%eax)
      247f:	67 66 8f 44 24 10    	popl   0x10(%esp)
      2485:	67 66 ff 70 08       	pushl  0x8(%eax)
      248a:	67 66 8f 44 24 14    	popl   0x14(%esp)
      2490:	67 66 ff 70 0c       	pushl  0xc(%eax)
      2495:	67 66 8f 44 24 18    	popl   0x18(%esp)
      249b:	36 80 3e fc 04 c0    	cmpb   $0xc0,%ss:0x4fc
      24a1:	74 19                	je     0x24bc
      24a3:	67 66 83 44 24 1c 10 	addl   $0x10,0x1c(%esp)
      24aa:	66 58                	pop    %eax
      24ac:	1f                   	pop    %ds
      24ad:	07                   	pop    %es
      24ae:	67 ff 74 24 10       	push   0x10(%esp)
      24b3:	67 81 24 24 ff fc    	andw   $0xfcff,(%esp)
      24b9:	9d                   	popf
      24ba:	66 cb                	lretl
      24bc:	50                   	push   %ax
      24bd:	8c c8                	mov    %cs,%ax
      24bf:	67 3b 44 24 0e       	cmp    0xe(%esp),%ax
      24c4:	58                   	pop    %ax
      24c5:	75 2c                	jne    0x24f3
      24c7:	1e                   	push   %ds
      24c8:	16                   	push   %ss
      24c9:	1f                   	pop    %ds
      24ca:	17                   	pop    %ss
      24cb:	66 94                	xchg   %eax,%esp
      24cd:	67 66 8d 64 24 04    	lea    0x4(%esp),%esp
      24d3:	67 66 ff 70 0c       	pushl  0xc(%eax)
      24d8:	67 66 ff 70 08       	pushl  0x8(%eax)
      24dd:	67 8e 40 06          	mov    0x6(%eax),%es
      24e1:	67 66 c5 00          	lds    (%eax),%eax
      24e5:	67 ff 74 24 10       	push   0x10(%esp)
      24ea:	67 81 24 24 ff fc    	andw   $0xfcff,(%esp)
      24f0:	9d                   	popf
      24f1:	66 cb                	lretl
      24f3:	67 c7 44 24 10 4e 35 	movw   $0x354e,0x10(%esp)
      24fa:	67 c7 44 24 12 00 00 	movw   $0x0,0x12(%esp)
      2501:	67 8c 4c 24 14       	mov    %cs,0x14(%esp)
      2506:	67 ff 74 24 18       	push   0x18(%esp)
      250b:	67 81 24 24 ff fc    	andw   $0xfcff,(%esp)
      2511:	9d                   	popf
      2512:	66 58                	pop    %eax
      2514:	1f                   	pop    %ds
      2515:	07                   	pop    %es
      2516:	66 cb                	lretl
      2518:	67 66 0f b2 24 24    	lss    (%esp),%esp
      251e:	67 66 8d 64 24 04    	lea    0x4(%esp),%esp
      2524:	66 9c                	pushfl
      2526:	67 66 8f 44 24 08    	popl   0x8(%esp)
      252c:	66 cf                	iretl
      252e:	66 0f b7 e4          	movzwl %sp,%esp
      2532:	66 50                	push   %eax
      2534:	8c c8                	mov    %cs,%ax
      2536:	67 3b 44 24 04       	cmp    0x4(%esp),%ax
      253b:	74 42                	je     0x257f
      253d:	0f a8                	push   %gs
      253f:	0f a0                	push   %fs
      2541:	06                   	push   %es
      2542:	1e                   	push   %ds
      2543:	67 66 c5 44 24 0c    	lds    0xc(%esp),%eax
      2549:	67 8b 40 fe          	mov    -0x2(%eax),%ax
      254d:	3c cd                	cmp    $0xcd,%al
      254f:	75 6c                	jne    0x25bd
      2551:	66 0f b6 c4          	movzbl %ah,%eax
      2555:	67 66 87 44 24 08    	xchg   %eax,0x8(%esp)
      255b:	66 50                	push   %eax
      255d:	66 54                	push   %esp
      255f:	e8 2a 01             	call   0x268c
      2562:	66 5c                	pop    %esp
      2564:	67 66 8f 44 24 08    	popl   0x8(%esp)
      256a:	1f                   	pop    %ds
      256b:	07                   	pop    %es
      256c:	0f a1                	pop    %fs
      256e:	0f a9                	pop    %gs
      2570:	8c c8                	mov    %cs,%ax
      2572:	67 3b 44 24 08       	cmp    0x8(%esp),%ax
      2577:	0f 84 23 00          	je     0x259e
      257b:	66 58                	pop    %eax
      257d:	66 cf                	iretl
      257f:	66 58                	pop    %eax
      2581:	66 0e                	pushl  %cs
      2583:	67 66 ff 74 24 04    	pushl  0x4(%esp)
      2589:	66 50                	push   %eax
      258b:	67 66 8d 44 24 18    	lea    0x18(%esp),%eax
      2591:	67 66 89 44 24 10    	mov    %eax,0x10(%esp)
      2597:	67 8c 54 24 14       	mov    %ss,0x14(%esp)
      259c:	eb 9f                	jmp    0x253d
      259e:	67 66 8b 44 24 0c    	mov    0xc(%esp),%eax
      25a4:	67 66 89 44 24 14    	mov    %eax,0x14(%esp)
      25aa:	67 8c 4c 24 10       	mov    %cs,0x10(%esp)
      25af:	66 58                	pop    %eax
      25b1:	67 66 8f 04 24       	popl   (%esp)
      25b6:	67 66 8f 04 24       	popl   (%esp)
      25bb:	66 cf                	iretl
      25bd:	80 fc cc             	cmp    $0xcc,%ah
      25c0:	b4 03                	mov    $0x3,%ah
      25c2:	74 8d                	je     0x2551
      25c4:	b4 04                	mov    $0x4,%ah
      25c6:	eb 89                	jmp    0x2551
      25c8:	80 fc cc             	cmp    $0xcc,%ah
      25cb:	8a c4                	mov    %ah,%al
      25cd:	b4 03                	mov    $0x3,%ah
      25cf:	0f 84 5a 00          	je     0x262d
      25d3:	3c ce                	cmp    $0xce,%al
      25d5:	b4 04                	mov    $0x4,%ah
      25d7:	0f 84 52 00          	je     0x262d
      25db:	b4 07                	mov    $0x7,%ah
      25dd:	eb 4e                	jmp    0x262d
      25df:	90                   	nop
      25e0:	66 1e                	pushl  %ds
      25e2:	66 50                	push   %eax
      25e4:	66 8b c4             	mov    %esp,%eax
      25e7:	16                   	push   %ss
      25e8:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      25ed:	81 2e 14 01 80 01    	subw   $0x180,0x114
      25f3:	1f                   	pop    %ds
      25f4:	2e 8e 16 12 10       	mov    %cs:0x1012,%ss
      25f9:	36 66 8b 26 14 01    	mov    %ss:0x114,%esp
      25ff:	66 1e                	pushl  %ds
      2601:	66 50                	push   %eax
      2603:	67 66 83 04 24 14    	addl   $0x14,(%esp)
      2609:	67 66 ff 70 10       	pushl  0x10(%eax)
      260e:	67 66 ff 70 0c       	pushl  0xc(%eax)
      2613:	67 66 ff 70 08       	pushl  0x8(%eax)
      2618:	67 66 ff 30          	pushl  (%eax)
      261c:	67 ff 70 04          	push   0x4(%eax)
      2620:	67 66 c5 40 08       	lds    0x8(%eax),%eax
      2625:	67 8b 40 fe          	mov    -0x2(%eax),%ax
      2629:	3c cd                	cmp    $0xcd,%al
      262b:	75 9b                	jne    0x25c8
      262d:	66 0f b6 c4          	movzbl %ah,%eax
      2631:	1f                   	pop    %ds
      2632:	67 66 87 04 24       	xchg   %eax,(%esp)
      2637:	0f a8                	push   %gs
      2639:	0f a0                	push   %fs
      263b:	06                   	push   %es
      263c:	1e                   	push   %ds
      263d:	66 50                	push   %eax
      263f:	66 54                	push   %esp
      2641:	fa                   	cli
      2642:	e8 47 00             	call   0x268c
      2645:	66 58                	pop    %eax
      2647:	16                   	push   %ss
      2648:	1f                   	pop    %ds
      2649:	67 66 0f b2 60 1c    	lss    0x1c(%eax),%esp
      264f:	81 06 14 01 80 01    	addw   $0x180,0x114
      2655:	67 66 ff 70 18       	pushl  0x18(%eax)
      265a:	67 f7 04 24 00 02    	testw  $0x200,(%esp)
      2660:	74 01                	je     0x2663
      2662:	fb                   	sti
      2663:	67 66 ff 70 14       	pushl  0x14(%eax)
      2668:	67 66 ff 70 10       	pushl  0x10(%eax)
      266d:	67 8e 68 0a          	mov    0xa(%eax),%gs
      2671:	67 8e 60 08          	mov    0x8(%eax),%fs
      2675:	67 8e 40 06          	mov    0x6(%eax),%es
      2679:	67 66 c5 00          	lds    (%eax),%eax
      267d:	66 cf                	iretl
      267f:	3d 38 00             	cmp    $0x38,%ax
      2682:	76 28                	jbe    0x26ac
      2684:	66 05 f4 be 00 00    	add    $0xbef4,%eax
      268a:	eb 26                	jmp    0x26b2
      268c:	2e 8e 1e 18 10       	mov    %cs:0x1018,%ds
      2691:	55                   	push   %bp
      2692:	8b ec                	mov    %sp,%bp
      2694:	66 57                	push   %edi
      2696:	66 0f b7 7e 04       	movzwl 0x4(%bp),%edi
      269b:	36 66 0f b6 45 0c    	movzbl %ss:0xc(%di),%eax
      26a1:	66 c1 e0 03          	shl    $0x3,%eax
      26a5:	36 f6 45 0e 02       	testb  $0x2,%ss:0xe(%di)
      26aa:	75 d3                	jne    0x267f
      26ac:	36 66 03 06 10 02    	add    %ss:0x210,%eax
      26b2:	67 ff 70 06          	push   0x6(%eax)
      26b6:	1e                   	push   %ds
      26b7:	66 50                	push   %eax
      26b9:	66 57                	push   %edi
      26bb:	36 66 ff 75 18       	pushl  %ss:0x18(%di)
      26c0:	67 81 24 24 ff bc    	andw   $0xbcff,(%esp)
      26c6:	67 81 0c 24 00 30    	orw    $0x3000,(%esp)
      26cc:	2e 66 ff 36 16 10    	pushl  %cs:0x1016
      26d2:	66 68 15 bf 00 00    	pushl  $0xbf15
      26d8:	36 80 3e fc 04 c0    	cmpb   $0xc0,%ss:0x4fc
      26de:	74 0d                	je     0x26ed
      26e0:	66 8b c4             	mov    %esp,%eax
      26e3:	67 66 ff 35 82 bf 00 	addr32 pushl 0xbf82
      26ea:	00 
      26eb:	66 50                	push   %eax
      26ed:	36 66 8b 45 18       	mov    %ss:0x18(%di),%eax
      26f2:	80 e4 bc             	and    $0xbc,%ah
      26f5:	80 cc 30             	or     $0x30,%ah
      26f8:	66 50                	push   %eax
      26fa:	66 8b 46 f4          	mov    -0xc(%bp),%eax
      26fe:	67 66 ff 70 04       	pushl  0x4(%eax)
      2703:	67 66 ff 30          	pushl  (%eax)
      2707:	67 66 8d 84 24 80 fe 	lea    -0x180(%esp),%eax
      270e:	ff ff 
      2710:	36 66 ff 36 14 01    	pushl  %ss:0x114
      2716:	24 fc                	and    $0xfc,%al
      2718:	36 66 a3 14 01       	mov    %eax,%ss:0x114
      271d:	36 67 66 8f 40 08    	popl   %ss:0x8(%eax)
      2723:	36 67 89 68 06       	mov    %bp,%ss:0x6(%eax)
      2728:	57                   	push   %di
      2729:	1e                   	push   %ds
      272a:	36 f6 45 0e 01       	testb  $0x1,%ss:0xe(%di)
      272f:	75 13                	jne    0x2744
      2731:	58                   	pop    %ax
      2732:	36 ff 75 04          	push   %ss:0x4(%di)
      2736:	36 8e 45 06          	mov    %ss:0x6(%di),%es
      273a:	36 8e 65 08          	mov    %ss:0x8(%di),%fs
      273e:	36 8e 6d 0a          	mov    %ss:0xa(%di),%gs
      2742:	33 ff                	xor    %di,%di
      2744:	66 8b 46 f4          	mov    -0xc(%bp),%eax
      2748:	67 89 78 06          	mov    %di,0x6(%eax)
      274c:	1f                   	pop    %ds
      274d:	5f                   	pop    %di
      274e:	36 66 8b 05          	mov    %ss:(%di),%eax
      2752:	66 8b 7e fc          	mov    -0x4(%bp),%edi
      2756:	8b 6e 00             	mov    0x0(%bp),%bp
      2759:	66 cf                	iretl
      275b:	66 0f b7 e4          	movzwl %sp,%esp
      275f:	67 66 8d 64 24 16    	lea    0x16(%esp),%esp
      2765:	67 87 2c 24          	xchg   %bp,(%esp)
      2769:	8f 46 00             	pop    0x0(%bp)
      276c:	66 89 7e fc          	mov    %edi,-0x4(%bp)
      2770:	66 8f 46 ec          	popl   -0x14(%bp)
      2774:	8d 7e ec             	lea    -0x14(%bp),%di
      2777:	8b e7                	mov    %di,%sp
      2779:	36 66 8f 06 14 01    	popl   %ss:0x114
      277f:	66 5f                	pop    %edi
      2781:	36 66 89 05          	mov    %eax,%ss:(%di)
      2785:	66 9c                	pushfl
      2787:	66 58                	pop    %eax
      2789:	36 81 65 18 00 03    	andw   $0x300,%ss:0x18(%di)
      278f:	80 e4 fc             	and    $0xfc,%ah
      2792:	36 09 45 18          	or     %ax,%ss:0x18(%di)
      2796:	8c c8                	mov    %cs,%ax
      2798:	36 3b 45 14          	cmp    %ss:0x14(%di),%ax
      279c:	74 17                	je     0x27b5
      279e:	36 f6 45 0e 01       	testb  $0x1,%ss:0xe(%di)
      27a3:	75 10                	jne    0x27b5
      27a5:	36 8c 5d 04          	mov    %ds,%ss:0x4(%di)
      27a9:	36 8c 45 06          	mov    %es,%ss:0x6(%di)
      27ad:	36 8c 65 08          	mov    %fs,%ss:0x8(%di)
      27b1:	36 8c 6d 0a          	mov    %gs,%ss:0xa(%di)
      27b5:	66 58                	pop    %eax
      27b7:	1f                   	pop    %ds
      27b8:	67 8f 40 06          	pop    0x6(%eax)
      27bc:	66 5f                	pop    %edi
      27be:	5d                   	pop    %bp
      27bf:	c3                   	ret
      27c0:	67 89 6c 24 18       	mov    %bp,0x18(%esp)
      27c5:	36 8b 2e 14 01       	mov    %ss:0x114,%bp
      27ca:	66 ff 76 08          	pushl  0x8(%bp)
      27ce:	8b 6e 06             	mov    0x6(%bp),%bp
      27d1:	eb 99                	jmp    0x276c
      27d3:	6a 00                	push   $0x0
      27d5:	eb 48                	jmp    0x281f
      27d7:	6a 01                	push   $0x1
      27d9:	eb 44                	jmp    0x281f
      27db:	6a 02                	push   $0x2
      27dd:	eb 40                	jmp    0x281f
      27df:	6a 03                	push   $0x3
      27e1:	eb 3c                	jmp    0x281f
      27e3:	6a 04                	push   $0x4
      27e5:	eb 38                	jmp    0x281f
      27e7:	6a 05                	push   $0x5
      27e9:	eb 34                	jmp    0x281f
      27eb:	6a 06                	push   $0x6
      27ed:	eb 30                	jmp    0x281f
      27ef:	6a 07                	push   $0x7
      27f1:	eb 2c                	jmp    0x281f
      27f3:	6a 08                	push   $0x8
      27f5:	eb 28                	jmp    0x281f
      27f7:	6a 09                	push   $0x9
      27f9:	eb 24                	jmp    0x281f
      27fb:	6a 0a                	push   $0xa
      27fd:	eb 20                	jmp    0x281f
      27ff:	6a 0b                	push   $0xb
      2801:	eb 1c                	jmp    0x281f
      2803:	6a 0c                	push   $0xc
      2805:	eb 18                	jmp    0x281f
      2807:	6a 0d                	push   $0xd
      2809:	eb 14                	jmp    0x281f
      280b:	6a 0e                	push   $0xe
      280d:	eb 10                	jmp    0x281f
      280f:	6a 0f                	push   $0xf
      2811:	eb 0c                	jmp    0x281f
      2813:	6a 14                	push   $0x14
      2815:	eb 08                	jmp    0x281f
      2817:	68 bb 00             	push   $0xbb
      281a:	eb 03                	jmp    0x281f
      281c:	68 bc 00             	push   $0xbc
      281f:	50                   	push   %ax
      2820:	8c c8                	mov    %cs,%ax
      2822:	3d 08 00             	cmp    $0x8,%ax
      2825:	0f 85 b5 00          	jne    0x28de
      2829:	66 0f b7 e4          	movzwl %sp,%esp
      282d:	67 3b 44 24 08       	cmp    0x8(%esp),%ax
      2832:	74 3f                	je     0x2873
      2834:	58                   	pop    %ax
      2835:	50                   	push   %ax
      2836:	0f a8                	push   %gs
      2838:	0f a0                	push   %fs
      283a:	06                   	push   %es
      283b:	1e                   	push   %ds
      283c:	66 50                	push   %eax
      283e:	66 54                	push   %esp
      2840:	67 66 0f b6 44 24 12 	movzbl 0x12(%esp),%eax
      2847:	04 68                	add    $0x68,%al
      2849:	a8 08                	test   $0x8,%al
      284b:	74 02                	je     0x284f
      284d:	2c 60                	sub    $0x60,%al
      284f:	67 66 89 44 24 10    	mov    %eax,0x10(%esp)
      2855:	e8 34 fe             	call   0x268c
      2858:	66 58                	pop    %eax
      285a:	67 66 8f 44 24 08    	popl   0x8(%esp)
      2860:	1f                   	pop    %ds
      2861:	07                   	pop    %es
      2862:	0f a1                	pop    %fs
      2864:	0f a9                	pop    %gs
      2866:	8c c8                	mov    %cs,%ax
      2868:	67 3b 44 24 08       	cmp    0x8(%esp),%ax
      286d:	74 38                	je     0x28a7
      286f:	66 58                	pop    %eax
      2871:	66 cf                	iretl
      2873:	67 66 ff 74 24 04    	pushl  0x4(%esp)
      2879:	67 66 ff 74 24 04    	pushl  0x4(%esp)
      287f:	66 50                	push   %eax
      2881:	67 8c 4c 24 0c       	mov    %cs,0xc(%esp)
      2886:	67 66 ff 74 24 18    	pushl  0x18(%esp)
      288c:	67 66 8f 44 24 10    	popl   0x10(%esp)
      2892:	67 8c 54 24 18       	mov    %ss,0x18(%esp)
      2897:	67 66 8d 44 24 1c    	lea    0x1c(%esp),%eax
      289d:	67 66 89 44 24 14    	mov    %eax,0x14(%esp)
      28a3:	66 58                	pop    %eax
      28a5:	eb 8d                	jmp    0x2834
      28a7:	66 1e                	pushl  %ds
      28a9:	67 66 c5 44 24 14    	lds    0x14(%esp),%eax
      28af:	67 66 ff 74 24 10    	pushl  0x10(%esp)
      28b5:	67 66 8f 40 fc       	popl   -0x4(%eax)
      28ba:	67 8c 48 f8          	mov    %cs,-0x8(%eax)
      28be:	67 66 ff 74 24 08    	pushl  0x8(%esp)
      28c4:	67 66 8f 40 f4       	popl   -0xc(%eax)
      28c9:	67 66 8f 40 f0       	popl   -0x10(%eax)
      28ce:	67 66 8f 40 ec       	popl   -0x14(%eax)
      28d3:	67 66 8d 60 ec       	lea    -0x14(%eax),%esp
      28d8:	66 58                	pop    %eax
      28da:	66 1f                	popl   %ds
      28dc:	66 cf                	iretl
      28de:	58                   	pop    %ax
      28df:	1e                   	push   %ds
      28e0:	66 50                	push   %eax
      28e2:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      28e7:	81 2e 14 01 80 01    	subw   $0x180,0x114
      28ed:	1e                   	push   %ds
      28ee:	16                   	push   %ss
      28ef:	66 a1 14 01          	mov    0x114,%eax
      28f3:	1f                   	pop    %ds
      28f4:	17                   	pop    %ss
      28f5:	66 94                	xchg   %eax,%esp
      28f7:	66 1e                	pushl  %ds
      28f9:	66 50                	push   %eax
      28fb:	67 66 ff 70 10       	pushl  0x10(%eax)
      2900:	67 66 ff 70 0c       	pushl  0xc(%eax)
      2905:	67 66 ff 70 08       	pushl  0x8(%eax)
      290a:	67 66 ff 30          	pushl  (%eax)
      290e:	67 ff 70 06          	push   0x6(%eax)
      2912:	2e 8e 1e 18 10       	mov    %cs:0x1018,%ds
      2917:	66 33 c0             	xor    %eax,%eax
      291a:	58                   	pop    %ax
      291b:	04 68                	add    $0x68,%al
      291d:	a8 08                	test   $0x8,%al
      291f:	0f 84 02 00          	je     0x2925
      2923:	2c 60                	sub    $0x60,%al
      2925:	67 66 87 04 24       	xchg   %eax,(%esp)
      292a:	0f a8                	push   %gs
      292c:	0f a0                	push   %fs
      292e:	06                   	push   %es
      292f:	1e                   	push   %ds
      2930:	66 50                	push   %eax
      2932:	66 54                	push   %esp
      2934:	e8 55 fd             	call   0x268c
      2937:	66 58                	pop    %eax
      2939:	67 66 c5 44 24 1c    	lds    0x1c(%esp),%eax
      293f:	67 66 8f 00          	popl   (%eax)
      2943:	66 83 c4 02          	add    $0x2,%esp
      2947:	67 8f 40 06          	pop    0x6(%eax)
      294b:	0f a1                	pop    %fs
      294d:	0f a9                	pop    %gs
      294f:	67 66 8d 64 24 04    	lea    0x4(%esp),%esp
      2955:	67 66 8f 40 08       	popl   0x8(%eax)
      295a:	67 66 8f 40 0c       	popl   0xc(%eax)
      295f:	67 66 8f 40 10       	popl   0x10(%eax)
      2964:	16                   	push   %ss
      2965:	1f                   	pop    %ds
      2966:	67 66 0f b2 24 24    	lss    (%esp),%esp
      296c:	81 06 14 01 80 01    	addw   $0x180,0x114
      2972:	66 58                	pop    %eax
      2974:	1f                   	pop    %ds
      2975:	07                   	pop    %es
      2976:	fb                   	sti
      2977:	66 cf                	iretl
      2979:	66 33 c9             	xor    %ecx,%ecx
      297c:	66 39 0e 98 01       	cmp    %ecx,0x198
      2981:	74 0c                	je     0x298f
      2983:	49                   	dec    %cx
      2984:	b4 05                	mov    $0x5,%ah
      2986:	ff 1e 98 01          	lcall  *0x198
      298a:	48                   	dec    %ax
      298b:	74 1a                	je     0x29a7
      298d:	e2 f5                	loop   0x2984
      298f:	80 3e fc 04 c4       	cmpb   $0xc4,0x4fc
      2994:	74 11                	je     0x29a7
      2996:	e8 24 07             	call   0x30bd
      2999:	72 0c                	jb     0x29a7
      299b:	66 33 c9             	xor    %ecx,%ecx
      299e:	88 0e 60 05          	mov    %cl,0x560
      29a2:	b1 01                	mov    $0x1,%cl
      29a4:	e8 85 06             	call   0x302c
      29a7:	66 0f b7 0e 8a 01    	movzwl 0x18a,%ecx
      29ad:	e3 61                	jcxz   0x2a10
      29af:	b8 00 ff             	mov    $0xff00,%ax
      29b2:	8e c0                	mov    %ax,%es
      29b4:	26 66 81 3e 03 10 56 	cmpl   $0x53494456,%es:0x1003
      29bb:	44 49 53 
      29be:	75 15                	jne    0x29d5
      29c0:	26 80 3e 07 10 4b    	cmpb   $0x4b,%es:0x1007
      29c6:	75 0d                	jne    0x29d5
      29c8:	26 a1 1e 10          	mov    %es:0x101e,%ax
      29cc:	3b 06 88 01          	cmp    0x188,%ax
      29d0:	7c 03                	jl     0x29d5
      29d2:	a3 88 01             	mov    %ax,0x188
      29d5:	66 0f b7 06 88 01    	movzwl 0x188,%eax
      29db:	f7 d8                	neg    %ax
      29dd:	24 fc                	and    $0xfc,%al
      29df:	f7 d8                	neg    %ax
      29e1:	a3 88 01             	mov    %ax,0x188
      29e4:	66 81 c1 00 04 00 00 	add    $0x400,%ecx
      29eb:	66 2b c8             	sub    %eax,%ecx
      29ee:	77 08                	ja     0x29f8
      29f0:	c7 06 8a 01 00 00    	movw   $0x0,0x18a
      29f6:	eb 18                	jmp    0x2a10
      29f8:	b8 15 35             	mov    $0x3515,%ax
      29fb:	cd 21                	int    $0x21
      29fd:	1e                   	push   %ds
      29fe:	0e                   	push   %cs
      29ff:	1f                   	pop    %ds
      2a00:	06                   	push   %es
      2a01:	53                   	push   %bx
      2a02:	66 8f 06 7a 41       	popl   0x417a
      2a07:	b8 15 25             	mov    $0x2515,%ax
      2a0a:	ba 69 41             	mov    $0x4169,%dx
      2a0d:	cd 21                	int    $0x21
      2a0f:	1f                   	pop    %ds
      2a10:	c3                   	ret
      2a11:	8c 06 f4 04          	mov    %es,0x4f4
      2a15:	8c c6                	mov    %es,%si
      2a17:	8e de                	mov    %si,%ds
      2a19:	c1 ee 06             	shr    $0x6,%si
      2a1c:	81 c6 00 10          	add    $0x1000,%si
      2a20:	fc                   	cld
      2a21:	66 ad                	lods   %ds:(%si),%eax
      2a23:	25 00 f0             	and    $0xf000,%ax
      2a26:	36 66 a3 2c 01       	mov    %eax,%ss:0x12c
      2a2b:	36 66 a3 e4 05       	mov    %eax,%ss:0x5e4
      2a30:	33 ff                	xor    %di,%di
      2a32:	66 a5                	movsl  %ds:(%si),%es:(%di)
      2a34:	16                   	push   %ss
      2a35:	1f                   	pop    %ds
      2a36:	b0 07                	mov    $0x7,%al
      2a38:	66 ab                	stos   %eax,%es:(%di)
      2a3a:	e8 01 00             	call   0x2a3e
      2a3d:	c3                   	ret
      2a3e:	66 c7 06 e4 04 00 00 	movl   $0x0,0x4e4
      2a45:	00 00 
      2a47:	80 3e fc 04 c4       	cmpb   $0xc4,0x4fc
      2a4c:	75 3b                	jne    0x2a89
      2a4e:	66 0f b7 06 00 05    	movzwl 0x500,%eax
      2a54:	0b c0                	or     %ax,%ax
      2a56:	74 31                	je     0x2a89
      2a58:	66 c1 e0 0e          	shl    $0xe,%eax
      2a5c:	66 01 06 e4 04       	add    %eax,0x4e4
      2a61:	66 a1 e0 04          	mov    0x4e0,%eax
      2a65:	66 01 06 e4 04       	add    %eax,0x4e4
      2a6a:	81 3e f6 04 b9 3b    	cmpw   $0x3bb9,0x4f6
      2a70:	77 17                	ja     0x2a89
      2a72:	b4 42                	mov    $0x42,%ah
      2a74:	cd 67                	int    $0x67
      2a76:	0a e4                	or     %ah,%ah
      2a78:	0f 85 0d 00          	jne    0x2a89
      2a7c:	66 0f b7 db          	movzwl %bx,%ebx
      2a80:	66 c1 e3 0e          	shl    $0xe,%ebx
      2a84:	66 01 1e e4 04       	add    %ebx,0x4e4
      2a89:	66 a1 8e 01          	mov    0x18e,%eax
      2a8d:	66 2b 06 92 01       	sub    0x192,%eax
      2a92:	66 c1 e0 0a          	shl    $0xa,%eax
      2a96:	66 01 06 e4 04       	add    %eax,0x4e4
      2a9b:	66 0f b7 06 8c 01    	movzwl 0x18c,%eax
      2aa1:	66 c1 e0 10          	shl    $0x10,%eax
      2aa5:	66 01 06 e4 04       	add    %eax,0x4e4
      2aaa:	66 0f b7 06 8a 01    	movzwl 0x18a,%eax
      2ab0:	66 05 00 04 00 00    	add    $0x400,%eax
      2ab6:	66 0f b7 1e 88 01    	movzwl 0x188,%ebx
      2abc:	66 2b c3             	sub    %ebx,%eax
      2abf:	0f 82 09 00          	jb     0x2acc
      2ac3:	66 c1 e0 0a          	shl    $0xa,%eax
      2ac7:	66 01 06 e4 04       	add    %eax,0x4e4
      2acc:	66 a1 c4 04          	mov    0x4c4,%eax
      2ad0:	66 2b 06 c0 04       	sub    0x4c0,%eax
      2ad5:	66 01 06 e4 04       	add    %eax,0x4e4
      2ada:	e8 01 00             	call   0x2ade
      2add:	c3                   	ret
      2ade:	66 8b 0e e4 04       	mov    0x4e4,%ecx
      2ae3:	66 ba 00 00 00 e0    	mov    $0xe0000000,%edx
      2ae9:	66 2b 0e ec 04       	sub    0x4ec,%ecx
      2aee:	72 34                	jb     0x2b24
      2af0:	66 8b c1             	mov    %ecx,%eax
      2af3:	66 c1 e8 14          	shr    $0x14,%eax
      2af7:	66 83 c0 02          	add    $0x2,%eax
      2afb:	66 c1 e0 0c          	shl    $0xc,%eax
      2aff:	66 2b c8             	sub    %eax,%ecx
      2b02:	66 3b ca             	cmp    %edx,%ecx
      2b05:	72 03                	jb     0x2b0a
      2b07:	66 8b ca             	mov    %edx,%ecx
      2b0a:	66 2b 0e d0 04       	sub    0x4d0,%ecx
      2b0f:	76 13                	jbe    0x2b24
      2b11:	66 2b 16 d4 04       	sub    0x4d4,%edx
      2b16:	66 2b ca             	sub    %edx,%ecx
      2b19:	72 09                	jb     0x2b24
      2b1b:	66 89 0e d8 04       	mov    %ecx,0x4d8
      2b20:	66 8b c1             	mov    %ecx,%eax
      2b23:	c3                   	ret
      2b24:	66 33 c9             	xor    %ecx,%ecx
      2b27:	eb f2                	jmp    0x2b1b
      2b29:	66 60                	pushal
      2b2b:	66 8b 16 f8 04       	mov    0x4f8,%edx
      2b30:	ff 26 f6 04          	jmp    *0x4f6
      2b34:	80 3e fc 04 c4       	cmpb   $0xc4,0x4fc
      2b39:	75 6c                	jne    0x2ba7
      2b3b:	83 3e 00 05 00       	cmpw   $0x0,0x500
      2b40:	74 3b                	je     0x2b7d
      2b42:	c7 06 f6 04 8e 3b    	movw   $0x3b8e,0x4f6
      2b48:	b4 41                	mov    $0x41,%ah
      2b4a:	cd 67                	int    $0x67
      2b4c:	c1 eb 06             	shr    $0x6,%bx
      2b4f:	81 c3 00 10          	add    $0x1000,%bx
      2b53:	53                   	push   %bx
      2b54:	6a 10                	push   $0x10
      2b56:	66 5a                	pop    %edx
      2b58:	66 0f a4 d6 10       	shld   $0x10,%edx,%esi
      2b5d:	66 0f b7 f6          	movzwl %si,%esi
      2b61:	80 ea 04             	sub    $0x4,%dl
      2b64:	72 17                	jb     0x2b7d
      2b66:	03 f2                	add    %dx,%si
      2b68:	66 03 36 f0 04       	add    0x4f0,%esi
      2b6d:	1e                   	push   %ds
      2b6e:	2e 8e 1e 14 10       	mov    %cs:0x1014,%ds
      2b73:	67 66 ad             	lods   %ds:(%esi),%eax
      2b76:	1f                   	pop    %ds
      2b77:	80 e4 f0             	and    $0xf0,%ah
      2b7a:	e9 d1 00             	jmp    0x2c4e
      2b7d:	c7 06 f6 04 b9 3b    	movw   $0x3bb9,0x4f6
      2b83:	b8 04 de             	mov    $0xde04,%ax
      2b86:	66 2e ff 1e 22 10    	lcalll *%cs:0x1022
      2b8c:	0a e4                	or     %ah,%ah
      2b8e:	0f 85 15 00          	jne    0x2ba7
      2b92:	66 81 06 e0 04 00 10 	addl   $0x1000,0x4e0
      2b99:	00 00 
      2b9b:	66 8b c2             	mov    %edx,%eax
      2b9e:	80 e4 f0             	and    $0xf0,%ah
      2ba1:	80 cc 08             	or     $0x8,%ah
      2ba4:	e9 a7 00             	jmp    0x2c4e
      2ba7:	c7 06 f6 04 e8 3b    	movw   $0x3be8,0x4f6
      2bad:	66 8b 16 92 01       	mov    0x192,%edx
      2bb2:	66 3b 16 8e 01       	cmp    0x18e,%edx
      2bb7:	73 0e                	jae    0x2bc7
      2bb9:	66 8b c2             	mov    %edx,%eax
      2bbc:	66 83 c2 04          	add    $0x4,%edx
      2bc0:	66 c1 e0 0a          	shl    $0xa,%eax
      2bc4:	e9 87 00             	jmp    0x2c4e
      2bc7:	f6 06 8c 01 01       	testb  $0x1,0x18c
      2bcc:	74 1d                	je     0x2beb
      2bce:	c7 06 f6 04 10 3c    	movw   $0x3c10,0x4f6
      2bd4:	66 ba 00 00 10 00    	mov    $0x100000,%edx
      2bda:	0a d2                	or     %dl,%dl
      2bdc:	0f 85 0b 00          	jne    0x2beb
      2be0:	66 8b c2             	mov    %edx,%eax
      2be3:	81 c2 00 10          	add    $0x1000,%dx
      2be7:	1a d2                	sbb    %dl,%dl
      2be9:	eb 63                	jmp    0x2c4e
      2beb:	c7 06 f6 04 34 3c    	movw   $0x3c34,0x4f6
      2bf1:	66 0f b7 16 8a 01    	movzwl 0x18a,%edx
      2bf7:	66 81 c2 00 04 00 00 	add    $0x400,%edx
      2bfe:	66 83 ea 04          	sub    $0x4,%edx
      2c02:	66 0f b7 06 88 01    	movzwl 0x188,%eax
      2c08:	66 3b d0             	cmp    %eax,%edx
      2c0b:	7c 09                	jl     0x2c16
      2c0d:	66 8b c2             	mov    %edx,%eax
      2c10:	66 c1 e0 0a          	shl    $0xa,%eax
      2c14:	eb 38                	jmp    0x2c4e
      2c16:	c7 06 f6 04 57 3c    	movw   $0x3c57,0x4f6
      2c1c:	66 8b 16 c0 04       	mov    0x4c0,%edx
      2c21:	66 3b 16 c4 04       	cmp    0x4c4,%edx
      2c26:	0f 83 6a 00          	jae    0x2c94
      2c2a:	66 8b f2             	mov    %edx,%esi
      2c2d:	66 81 c2 00 10 00 00 	add    $0x1000,%edx
      2c34:	1e                   	push   %ds
      2c35:	66 c1 ee 0a          	shr    $0xa,%esi
      2c39:	81 c6 00 10          	add    $0x1000,%si
      2c3d:	66 03 36 f0 04       	add    0x4f0,%esi
      2c42:	2e 8e 1e 14 10       	mov    %cs:0x1014,%ds
      2c47:	67 66 ad             	lods   %ds:(%esi),%eax
      2c4a:	1f                   	pop    %ds
      2c4b:	80 e4 f0             	and    $0xf0,%ah
      2c4e:	66 81 06 dc 04 00 10 	addl   $0x1000,0x4dc
      2c55:	00 00 
      2c57:	66 89 16 f8 04       	mov    %edx,0x4f8
      2c5c:	b0 07                	mov    $0x7,%al
      2c5e:	66 8b cf             	mov    %edi,%ecx
      2c61:	67 66 ab             	stos   %eax,%es:(%edi)
      2c64:	0f 20 d8             	mov    %cr3,%eax
      2c67:	0f 22 d8             	mov    %eax,%cr3
      2c6a:	66 81 f9 00 00 44 00 	cmp    $0x440000,%ecx
      2c71:	73 1d                	jae    0x2c90
      2c73:	67 66 8d b9 00 00 c0 	lea    -0x400000(%ecx),%edi
      2c7a:	ff 
      2c7b:	66 b8 02 00 00 00    	mov    $0x2,%eax
      2c81:	66 c1 e7 0a          	shl    $0xa,%edi
      2c85:	66 b9 00 04 00 00    	mov    $0x400,%ecx
      2c8b:	fc                   	cld
      2c8c:	67 f3 66 ab          	rep stos %eax,%es:(%edi)
      2c90:	66 61                	popal
      2c92:	f8                   	clc
      2c93:	c3                   	ret
      2c94:	66 61                	popal
      2c96:	66 33 c0             	xor    %eax,%eax
      2c99:	f9                   	stc
      2c9a:	c3                   	ret
      2c9b:	fc                   	cld
      2c9c:	66 57                	push   %edi
      2c9e:	67 8b 7c 24 06       	mov    0x6(%esp),%di
      2ca3:	0f 20 d0             	mov    %cr2,%eax
      2ca6:	66 2b 06 cc 04       	sub    0x4cc,%eax
      2cab:	0f 82 b0 00          	jb     0x2d5f
      2caf:	66 3b 06 d0 04       	cmp    0x4d0,%eax
      2cb4:	73 4a                	jae    0x2d00
      2cb6:	0f 20 d7             	mov    %cr2,%edi
      2cb9:	66 c1 ef 0a          	shr    $0xa,%edi
      2cbd:	83 e7 fc             	and    $0xfffc,%di
      2cc0:	66 81 c7 00 00 40 00 	add    $0x400000,%edi
      2cc7:	2e 8e 06 14 10       	mov    %cs:0x1014,%es
      2ccc:	e8 5a fe             	call   0x2b29
      2ccf:	0f 82 7b 00          	jb     0x2d4e
      2cd3:	66 5f                	pop    %edi
      2cd5:	58                   	pop    %ax
      2cd6:	5c                   	pop    %sp
      2cd7:	8c c8                	mov    %cs,%ax
      2cd9:	67 3b 44 24 14       	cmp    0x14(%esp),%ax
      2cde:	66 58                	pop    %eax
      2ce0:	1f                   	pop    %ds
      2ce1:	07                   	pop    %es
      2ce2:	0f a1                	pop    %fs
      2ce4:	0f a9                	pop    %gs
      2ce6:	67 66 8d 64 24 04    	lea    0x4(%esp),%esp
      2cec:	74 02                	je     0x2cf0
      2cee:	66 cf                	iretl
      2cf0:	67 66 8f 44 24 08    	popl   0x8(%esp)
      2cf6:	67 66 8f 44 24 08    	popl   0x8(%esp)
      2cfc:	66 9d                	popfl
      2cfe:	66 cb                	lretl
      2d00:	66 3d 00 00 00 e0    	cmp    $0xe0000000,%eax
      2d06:	73 4c                	jae    0x2d54
      2d08:	66 3b 06 d4 04       	cmp    0x4d4,%eax
      2d0d:	73 a7                	jae    0x2cb6
      2d0f:	66 50                	push   %eax
      2d11:	66 2b 06 d4 04       	sub    0x4d4,%eax
      2d16:	66 05 00 00 01 00    	add    $0x10000,%eax
      2d1c:	66 58                	pop    %eax
      2d1e:	72 0f                	jb     0x2d2f
      2d20:	66 50                	push   %eax
      2d22:	67 66 8d 40 04       	lea    0x4(%eax),%eax
      2d27:	66 3b 45 1c          	cmp    0x1c(%di),%eax
      2d2b:	66 58                	pop    %eax
      2d2d:	72 25                	jb     0x2d54
      2d2f:	25 00 f0             	and    $0xf000,%ax
      2d32:	66 2b 06 d4 04       	sub    0x4d4,%eax
      2d37:	66 f7 d8             	neg    %eax
      2d3a:	66 3b 06 d8 04       	cmp    0x4d8,%eax
      2d3f:	77 13                	ja     0x2d54
      2d41:	66 29 06 d8 04       	sub    %eax,0x4d8
      2d46:	66 29 06 d4 04       	sub    %eax,0x4d4
      2d4b:	e9 68 ff             	jmp    0x2cb6
      2d4e:	c7 06 40 02 42 02    	movw   $0x242,0x240
      2d54:	66 5f                	pop    %edi
      2d56:	c3                   	ret
      2d57:	c7 06 40 02 8e 02    	movw   $0x28e,0x240
      2d5d:	eb f5                	jmp    0x2d54
      2d5f:	0f 20 d0             	mov    %cr2,%eax
      2d62:	66 3d 00 00 40 00    	cmp    $0x400000,%eax
      2d68:	72 ea                	jb     0x2d54
      2d6a:	66 3d 00 00 80 00    	cmp    $0x800000,%eax
      2d70:	77 e2                	ja     0x2d54
      2d72:	e9 41 ff             	jmp    0x2cb6
      2d75:	80 3e fc 04 c4       	cmpb   $0xc4,0x4fc
      2d7a:	74 12                	je     0x2d8e
      2d7c:	66 33 c9             	xor    %ecx,%ecx
      2d7f:	66 39 0e 98 01       	cmp    %ecx,0x198
      2d84:	74 08                	je     0x2d8e
      2d86:	b4 06                	mov    $0x6,%ah
      2d88:	ff 1e 98 01          	lcall  *0x198
      2d8c:	eb 0b                	jmp    0x2d99
      2d8e:	b1 00                	mov    $0x0,%cl
      2d90:	38 0e 60 05          	cmp    %cl,0x560
      2d94:	75 03                	jne    0x2d99
      2d96:	e8 93 02             	call   0x302c
      2d99:	2e 66 a1 7a 41       	mov    %cs:0x417a,%eax
      2d9e:	66 0b c0             	or     %eax,%eax
      2da1:	0f 84 0b 00          	je     0x2db0
      2da5:	1e                   	push   %ds
      2da6:	66 50                	push   %eax
      2da8:	5a                   	pop    %dx
      2da9:	1f                   	pop    %ds
      2daa:	b8 15 25             	mov    $0x2515,%ax
      2dad:	cd 21                	int    $0x21
      2daf:	1f                   	pop    %ds
      2db0:	66 83 3e 98 01 00    	cmpl   $0x0,0x198
      2db6:	74 1f                	je     0x2dd7
      2db8:	b4 0d                	mov    $0xd,%ah
      2dba:	8b 16 96 01          	mov    0x196,%dx
      2dbe:	ff 1e 98 01          	lcall  *0x198
      2dc2:	b4 0a                	mov    $0xa,%ah
      2dc4:	ff 1e 98 01          	lcall  *0x198
      2dc8:	80 3e 8c 01 00       	cmpb   $0x0,0x18c
      2dcd:	0f 84 06 00          	je     0x2dd7
      2dd1:	b4 02                	mov    $0x2,%ah
      2dd3:	ff 1e 98 01          	lcall  *0x198
      2dd7:	80 3e fc 04 c4       	cmpb   $0xc4,0x4fc
      2ddc:	0f 85 bc 00          	jne    0x2e9c
      2de0:	66 83 3e 2c 01 00    	cmpl   $0x0,0x12c
      2de6:	0f 84 a3 00          	je     0x2e8d
      2dea:	8c d5                	mov    %ss,%bp
      2dec:	66 c1 c5 10          	rol    $0x10,%ebp
      2df0:	8b ec                	mov    %sp,%bp
      2df2:	81 2e 14 01 80 01    	subw   $0x180,0x114
      2df8:	a1 14 01             	mov    0x114,%ax
      2dfb:	1e                   	push   %ds
      2dfc:	17                   	pop    %ss
      2dfd:	8b e0                	mov    %ax,%sp
      2dff:	66 0f b7 3e f4 04    	movzwl 0x4f4,%edi
      2e05:	66 c1 e7 04          	shl    $0x4,%edi
      2e09:	67 66 8d 77 08       	lea    0x8(%edi),%esi
      2e0e:	66 f7 df             	neg    %edi
      2e11:	66 81 c7 fc 0f 00 00 	add    $0xffc,%edi
      2e18:	fc                   	cld
      2e19:	2e ff 16 16 14       	call   *%cs:0x1416
      2e1e:	e8 f6 dd             	call   0xc17
      2e21:	2e a1 14 10          	mov    %cs:0x1014,%ax
      2e25:	8e c0                	mov    %ax,%es
      2e27:	8e d8                	mov    %ax,%ds
      2e29:	b9 fe 03             	mov    $0x3fe,%cx
      2e2c:	e3 53                	jcxz   0x2e81
      2e2e:	67 66 ad             	lods   %ds:(%esi),%eax
      2e31:	a8 01                	test   $0x1,%al
      2e33:	e1 f9                	loope  0x2e2e
      2e35:	74 4a                	je     0x2e81
      2e37:	66 50                	push   %eax
      2e39:	66 56                	push   %esi
      2e3b:	51                   	push   %cx
      2e3c:	67 66 8d 34 37       	lea    (%edi,%esi,1),%esi
      2e41:	66 c1 e6 0a          	shl    $0xa,%esi
      2e45:	b9 00 04             	mov    $0x400,%cx
      2e48:	67 66 ad             	lods   %ds:(%esi),%eax
      2e4b:	a8 01                	test   $0x1,%al
      2e4d:	75 1b                	jne    0x2e6a
      2e4f:	e2 f7                	loop   0x2e48
      2e51:	59                   	pop    %cx
      2e52:	66 5e                	pop    %esi
      2e54:	66 5a                	pop    %edx
      2e56:	f6 c6 08             	test   $0x8,%dh
      2e59:	74 d1                	je     0x2e2c
      2e5b:	81 e2 00 f0          	and    $0xf000,%dx
      2e5f:	b8 05 de             	mov    $0xde05,%ax
      2e62:	66 2e ff 1e 22 10    	lcalll *%cs:0x1022
      2e68:	eb c2                	jmp    0x2e2c
      2e6a:	f6 c4 08             	test   $0x8,%ah
      2e6d:	74 e0                	je     0x2e4f
      2e6f:	66 8b d0             	mov    %eax,%edx
      2e72:	81 e2 00 f0          	and    $0xf000,%dx
      2e76:	b8 05 de             	mov    $0xde05,%ax
      2e79:	66 2e ff 1e 22 10    	lcalll *%cs:0x1022
      2e7f:	eb ce                	jmp    0x2e4f
      2e81:	2e ff 16 14 14       	call   *%cs:0x1414
      2e86:	66 55                	push   %ebp
      2e88:	67 0f b2 24 24       	lss    (%esp),%sp
      2e8d:	83 3e 00 05 00       	cmpw   $0x0,0x500
      2e92:	74 08                	je     0x2e9c
      2e94:	8b 16 fe 04          	mov    0x4fe,%dx
      2e98:	b4 45                	mov    $0x45,%ah
      2e9a:	cd 67                	int    $0x67
      2e9c:	2e c6 06 ab 3d c3    	movb   $0xc3,%cs:0x3dab
      2ea2:	c3                   	ret
      2ea3:	6a ff                	push   $0xffff
      2ea5:	5a                   	pop    %dx
      2ea6:	c3                   	ret
      2ea7:	66 8b 16 c4 04       	mov    0x4c4,%edx
      2eac:	66 c1 ea 04          	shr    $0x4,%edx
      2eb0:	2b 16 7c 01          	sub    0x17c,%dx
      2eb4:	c3                   	ret
      2eb5:	c6 06 fd 04 01       	movb   $0x1,0x4fd
      2eba:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      2ebf:	74 e6                	je     0x2ea7
      2ec1:	66 8b 1e c0 04       	mov    0x4c0,%ebx
      2ec6:	81 3e f6 04 57 3c    	cmpw   $0x3c57,0x4f6
      2ecc:	77 d5                	ja     0x2ea3
      2ece:	75 05                	jne    0x2ed5
      2ed0:	66 8b 1e f8 04       	mov    0x4f8,%ebx
      2ed5:	66 c1 eb 04          	shr    $0x4,%ebx
      2ed9:	2b 1e 7c 01          	sub    0x17c,%bx
      2edd:	8e 06 7c 01          	mov    0x17c,%es
      2ee1:	53                   	push   %bx
      2ee2:	b4 4a                	mov    $0x4a,%ah
      2ee4:	cd 21                	int    $0x21
      2ee6:	81 3e f6 04 34 3c    	cmpw   $0x3c34,0x4f6
      2eec:	77 b7                	ja     0x2ea5
      2eee:	66 0f b7 16 8a 01    	movzwl 0x18a,%edx
      2ef4:	75 0c                	jne    0x2f02
      2ef6:	66 8b 16 f8 04       	mov    0x4f8,%edx
      2efb:	66 81 ea 00 04 00 00 	sub    $0x400,%edx
      2f02:	2e 89 16 a7 41       	mov    %dx,%cs:0x41a7
      2f07:	66 8b 0e 98 01       	mov    0x198,%ecx
      2f0c:	67 e3 6c             	jecxz  0x2f7b
      2f0f:	f6 06 8c 01 01       	testb  $0x1,0x18c
      2f14:	74 0e                	je     0x2f24
      2f16:	81 3e f6 04 10 3c    	cmpw   $0x3c10,0x4f6
      2f1c:	73 5d                	jae    0x2f7b
      2f1e:	b4 02                	mov    $0x2,%ah
      2f20:	ff 1e 98 01          	lcall  *0x198
      2f24:	81 3e f6 04 e8 3b    	cmpw   $0x3be8,0x4f6
      2f2a:	77 4f                	ja     0x2f7b
      2f2c:	9c                   	pushf
      2f2d:	b4 0c                	mov    $0xc,%ah
      2f2f:	8b 16 96 01          	mov    0x196,%dx
      2f33:	ff 1e 98 01          	lcall  *0x198
      2f37:	8b d3                	mov    %bx,%dx
      2f39:	89 1e 9c 01          	mov    %bx,0x19c
      2f3d:	f7 db                	neg    %bx
      2f3f:	81 e3 00 f0          	and    $0xf000,%bx
      2f43:	f7 db                	neg    %bx
      2f45:	2b da                	sub    %dx,%bx
      2f47:	c1 eb 0a             	shr    $0xa,%bx
      2f4a:	66 8b 16 92 01       	mov    0x192,%edx
      2f4f:	66 0f b7 db          	movzwl %bx,%ebx
      2f53:	66 2b d3             	sub    %ebx,%edx
      2f56:	66 8b da             	mov    %edx,%ebx
      2f59:	9d                   	popf
      2f5a:	75 05                	jne    0x2f61
      2f5c:	66 8b 1e f8 04       	mov    0x4f8,%ebx
      2f61:	66 2b da             	sub    %edx,%ebx
      2f64:	8b 16 96 01          	mov    0x196,%dx
      2f68:	b9 02 00             	mov    $0x2,%cx
      2f6b:	53                   	push   %bx
      2f6c:	b4 0d                	mov    $0xd,%ah
      2f6e:	ff 1e 98 01          	lcall  *0x198
      2f72:	e2 f8                	loop   0x2f6c
      2f74:	5b                   	pop    %bx
      2f75:	b4 0f                	mov    $0xf,%ah
      2f77:	ff 1e 98 01          	lcall  *0x198
      2f7b:	5a                   	pop    %dx
      2f7c:	c3                   	ret
      2f7d:	c6 06 fd 04 00       	movb   $0x0,0x4fd
      2f82:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      2f87:	74 f3                	je     0x2f7c
      2f89:	66 8b 1e c4 04       	mov    0x4c4,%ebx
      2f8e:	66 c1 eb 04          	shr    $0x4,%ebx
      2f92:	2b 1e 7c 01          	sub    0x17c,%bx
      2f96:	8e 06 7c 01          	mov    0x17c,%es
      2f9a:	b4 4a                	mov    $0x4a,%ah
      2f9c:	cd 21                	int    $0x21
      2f9e:	73 11                	jae    0x2fb1
      2fa0:	b4 4a                	mov    $0x4a,%ah
      2fa2:	cd 21                	int    $0x21
      2fa4:	03 1e 7c 01          	add    0x17c,%bx
      2fa8:	66 c1 e3 04          	shl    $0x4,%ebx
      2fac:	66 89 1e c4 04       	mov    %ebx,0x4c4
      2fb1:	2e c7 06 a7 41 00 00 	movw   $0x0,%cs:0x41a7
      2fb8:	66 8b 0e 98 01       	mov    0x198,%ecx
      2fbd:	67 e3 66             	jecxz  0x3026
      2fc0:	f6 06 8c 01 01       	testb  $0x1,0x18c
      2fc5:	74 14                	je     0x2fdb
      2fc7:	81 3e f6 04 10 3c    	cmpw   $0x3c10,0x4f6
      2fcd:	73 57                	jae    0x3026
      2fcf:	ba ff ff             	mov    $0xffff,%dx
      2fd2:	b4 01                	mov    $0x1,%ah
      2fd4:	ff 1e 98 01          	lcall  *0x198
      2fd8:	a3 8c 01             	mov    %ax,0x18c
      2fdb:	81 3e f6 04 e8 3b    	cmpw   $0x3be8,0x4f6
      2fe1:	77 43                	ja     0x3026
      2fe3:	b4 0f                	mov    $0xf,%ah
      2fe5:	66 8b 1e 8e 01       	mov    0x18e,%ebx
      2fea:	66 2b 1e 92 01       	sub    0x192,%ebx
      2fef:	8b 16 96 01          	mov    0x196,%dx
      2ff3:	52                   	push   %dx
      2ff4:	ff 1e 98 01          	lcall  *0x198
      2ff8:	5a                   	pop    %dx
      2ff9:	8b f0                	mov    %ax,%si
      2ffb:	b4 0c                	mov    $0xc,%ah
      2ffd:	ff 1e 98 01          	lcall  *0x198
      3001:	23 f0                	and    %ax,%si
      3003:	52                   	push   %dx
      3004:	53                   	push   %bx
      3005:	2b 1e 9c 01          	sub    0x19c,%bx
      3009:	03 f3                	add    %bx,%si
      300b:	66 5b                	pop    %ebx
      300d:	66 f7 db             	neg    %ebx
      3010:	81 e3 00 f0          	and    $0xf000,%bx
      3014:	66 f7 db             	neg    %ebx
      3017:	66 c1 eb 0a          	shr    $0xa,%ebx
      301b:	66 2b 1e 92 01       	sub    0x192,%ebx
      3020:	03 de                	add    %si,%bx
      3022:	66 4b                	dec    %ebx
      3024:	90                   	nop
      3025:	90                   	nop
      3026:	c3                   	ret
      3027:	ba 3c 05             	mov    $0x53c,%dx
      302a:	eb 75                	jmp    0x30a1
      302c:	51                   	push   %cx
      302d:	2e 8a 26 18 41       	mov    %cs:0x4118,%ah
      3032:	9e                   	sahf
      3033:	72 37                	jb     0x306c
      3035:	78 29                	js     0x3060
      3037:	74 41                	je     0x307a
      3039:	d0 d9                	rcr    $1,%cl
      303b:	ba 60 00             	mov    $0x60,%dx
      303e:	66 b8 00 d1 df ff    	mov    $0xffdfd100,%eax
      3044:	72 06                	jb     0x304c
      3046:	66 b8 00 d1 dd ff    	mov    $0xffddd100,%eax
      304c:	e8 5b 00             	call   0x30aa
      304f:	66 c1 e8 08          	shr    $0x8,%eax
      3053:	80 f2 04             	xor    $0x4,%dl
      3056:	ee                   	out    %al,(%dx)
      3057:	0a e4                	or     %ah,%ah
      3059:	75 f1                	jne    0x304c
      305b:	e8 4c 00             	call   0x30aa
      305e:	eb 2e                	jmp    0x308e
      3060:	0a c9                	or     %cl,%cl
      3062:	b0 df                	mov    $0xdf,%al
      3064:	75 02                	jne    0x3068
      3066:	b0 dd                	mov    $0xdd,%al
      3068:	e6 60                	out    %al,$0x60
      306a:	eb 22                	jmp    0x308e
      306c:	e4 92                	in     $0x92,%al
      306e:	0c 02                	or     $0x2,%al
      3070:	0a c9                	or     %cl,%cl
      3072:	75 02                	jne    0x3076
      3074:	24 fd                	and    $0xfd,%al
      3076:	e6 92                	out    %al,$0x92
      3078:	eb 14                	jmp    0x308e
      307a:	b0 dd                	mov    $0xdd,%al
      307c:	d0 e1                	shl    $1,%cl
      307e:	02 c1                	add    %cl,%al
      3080:	b2 02                	mov    $0x2,%dl
      3082:	e8 25 00             	call   0x30aa
      3085:	e6 64                	out    %al,$0x64
      3087:	e8 20 00             	call   0x30aa
      308a:	fe ca                	dec    %dl
      308c:	75 f7                	jne    0x3085
      308e:	e8 2c 00             	call   0x30bd
      3091:	58                   	pop    %ax
      3092:	9f                   	lahf
      3093:	32 e0                	xor    %al,%ah
      3095:	9e                   	sahf
      3096:	72 01                	jb     0x3099
      3098:	c3                   	ret
      3099:	b8 00 00             	mov    $0x0,%ax
      309c:	8e d8                	mov    %ax,%ds
      309e:	ba 02 05             	mov    $0x502,%dx
      30a1:	b4 09                	mov    $0x9,%ah
      30a3:	cd 21                	int    $0x21
      30a5:	6a 01                	push   $0x1
      30a7:	e9 e5 d6             	jmp    0x78f
      30aa:	66 b9 00 00 10 00    	mov    $0x100000,%ecx
      30b0:	e4 64                	in     $0x64,%al
      30b2:	a8 02                	test   $0x2,%al
      30b4:	67 e0 f9             	loopnel 0x30b0
      30b7:	66 0b c9             	or     %ecx,%ecx
      30ba:	74 dd                	je     0x3099
      30bc:	c3                   	ret
      30bd:	1e                   	push   %ds
      30be:	06                   	push   %es
      30bf:	50                   	push   %ax
      30c0:	33 c0                	xor    %ax,%ax
      30c2:	8e d8                	mov    %ax,%ds
      30c4:	48                   	dec    %ax
      30c5:	8e c0                	mov    %ax,%es
      30c7:	8a 26 00 00          	mov    0x0,%ah
      30cb:	fe cc                	dec    %ah
      30cd:	8a c4                	mov    %ah,%al
      30cf:	26 86 06 10 00       	xchg   %al,%es:0x10
      30d4:	2a 26 00 00          	sub    0x0,%ah
      30d8:	26 86 06 10 00       	xchg   %al,%es:0x10
      30dd:	9e                   	sahf
      30de:	58                   	pop    %ax
      30df:	07                   	pop    %es
      30e0:	1f                   	pop    %ds
      30e1:	c3                   	ret
      30e2:	01 66 60             	add    %sp,0x60(%bp)
      30e5:	8b d8                	mov    %ax,%bx
      30e7:	66 c1 cb 10          	ror    $0x10,%ebx
      30eb:	b9 02 00             	mov    $0x2,%cx
      30ee:	eb 08                	jmp    0x30f8
      30f0:	66 60                	pushal
      30f2:	66 8b d8             	mov    %eax,%ebx
      30f5:	b9 04 00             	mov    $0x4,%cx
      30f8:	66 c1 c3 08          	rol    $0x8,%ebx
      30fc:	8a c3                	mov    %bl,%al
      30fe:	e8 05 00             	call   0x3106
      3101:	e2 f5                	loop   0x30f8
      3103:	66 61                	popal
      3105:	c3                   	ret
      3106:	8a f0                	mov    %al,%dh
      3108:	b4 02                	mov    $0x2,%ah
      310a:	8a d6                	mov    %dh,%dl
      310c:	c0 ea 04             	shr    $0x4,%dl
      310f:	80 c2 30             	add    $0x30,%dl
      3112:	80 fa 3a             	cmp    $0x3a,%dl
      3115:	0f 8c 03 00          	jl     0x311c
      3119:	80 c2 07             	add    $0x7,%dl
      311c:	cd 21                	int    $0x21
      311e:	8a d6                	mov    %dh,%dl
      3120:	80 e2 0f             	and    $0xf,%dl
      3123:	80 c2 30             	add    $0x30,%dl
      3126:	80 fa 3a             	cmp    $0x3a,%dl
      3129:	0f 8c 03 00          	jl     0x3130
      312d:	80 c2 07             	add    $0x7,%dl
      3130:	cd 21                	int    $0x21
      3132:	c3                   	ret
      3133:	9c                   	pushf
      3134:	80 fc 88             	cmp    $0x88,%ah
      3137:	0f 84 34 00          	je     0x316f
      313b:	80 fc 87             	cmp    $0x87,%ah
      313e:	0f 84 06 00          	je     0x3148
      3142:	9d                   	popf
      3143:	ea 00 00 00 00       	ljmp   $0x0,$0x0
      3148:	2e ff 1e 7a 41       	lcall  *%cs:0x417a
      314d:	9c                   	pushf
      314e:	67 87 04 24          	xchg   %ax,(%esp)
      3152:	67 89 44 24 06       	mov    %ax,0x6(%esp)
      3157:	58                   	pop    %ax
      3158:	e8 62 ff             	call   0x30bd
      315b:	0f 82 14 00          	jb     0x3173
      315f:	66 50                	push   %eax
      3161:	66 51                	push   %ecx
      3163:	52                   	push   %dx
      3164:	b1 01                	mov    $0x1,%cl
      3166:	e8 c3 fe             	call   0x302c
      3169:	5a                   	pop    %dx
      316a:	66 59                	pop    %ecx
      316c:	66 58                	pop    %eax
      316e:	cf                   	iret
      316f:	9d                   	popf
      3170:	b8 00 00             	mov    $0x0,%ax
      3173:	cf                   	iret
      3174:	66 60                	pushal
      3176:	66 83 26 04 06 ff    	andl   $0xffffffff,0x604
      317c:	9c                   	pushf
      317d:	66 a3 04 06          	mov    %eax,0x604
      3181:	66 03 06 08 06       	add    0x608,%eax
      3186:	66 50                	push   %eax
      3188:	66 50                	push   %eax
      318a:	5a                   	pop    %dx
      318b:	59                   	pop    %cx
      318c:	b8 07 00             	mov    $0x7,%ax
      318f:	2e 8b 1e 1a 10       	mov    %cs:0x101a,%bx
      3194:	cd 31                	int    $0x31
      3196:	0f 82 c4 08          	jb     0x3a5e
      319a:	2e 8b 1e 16 10       	mov    %cs:0x1016,%bx
      319f:	cd 31                	int    $0x31
      31a1:	0f 82 b9 08          	jb     0x3a5e
      31a5:	2e 8b 1e 18 10       	mov    %cs:0x1018,%bx
      31aa:	cd 31                	int    $0x31
      31ac:	0f 82 ae 08          	jb     0x3a5e
      31b0:	8e c3                	mov    %bx,%es
      31b2:	66 58                	pop    %eax
      31b4:	66 a3 cc 04          	mov    %eax,0x4cc
      31b8:	9d                   	popf
      31b9:	74 0b                	je     0x31c6
      31bb:	66 f7 d8             	neg    %eax
      31be:	26 67 66 a3 07 08 00 	addr32 mov %eax,%es:0x807
      31c5:	00 
      31c6:	66 61                	popal
      31c8:	c3                   	ret
      31c9:	b4 c0                	mov    $0xc0,%ah
      31cb:	cd 15                	int    $0x15
      31cd:	26 f6 47 05 02       	testb  $0x2,%es:0x5(%bx)
      31d2:	75 37                	jne    0x320b
      31d4:	2e c6 06 18 41 40    	movb   $0x40,%cs:0x4118
      31da:	68 00 f0             	push   $0xf000
      31dd:	07                   	pop    %es
      31de:	26 81 3e f8 00 48 50 	cmpw   $0x5048,%es:0xf8
      31e5:	74 24                	je     0x320b
      31e7:	2e c6 06 18 41 80    	movb   $0x80,%cs:0x4118
      31ed:	bb 00 f0             	mov    $0xf000,%bx
      31f0:	8e c3                	mov    %bx,%es
      31f2:	26 8a 1e fe ff       	mov    %es:-0x2,%bl
      31f7:	80 fb fb             	cmp    $0xfb,%bl
      31fa:	0f 84 0d 00          	je     0x320b
      31fe:	80 c3 02             	add    $0x2,%bl
      3201:	0f 82 06 00          	jb     0x320b
      3205:	2e c6 06 18 41 00    	movb   $0x0,%cs:0x4118
      320b:	c3                   	ret
      320c:	66 8b 16 14 01       	mov    0x114,%edx
      3211:	66 c1 ea 04          	shr    $0x4,%edx
      3215:	83 c2 02             	add    $0x2,%dx
      3218:	66 33 db             	xor    %ebx,%ebx
      321b:	8c d3                	mov    %ss,%bx
      321d:	03 d3                	add    %bx,%dx
      321f:	8e c2                	mov    %dx,%es
      3221:	89 16 04 02          	mov    %dx,0x204
      3225:	81 ea 6f 00          	sub    $0x6f,%dx
      3229:	66 c1 e2 04          	shl    $0x4,%edx
      322d:	89 16 06 02          	mov    %dx,0x206
      3231:	67 66 8d 42 03       	lea    0x3(%edx),%eax
      3236:	2e a3 3a 2c          	mov    %ax,%cs:0x2c3a
      323a:	66 81 fa 00 fc 00 00 	cmp    $0xfc00,%edx
      3241:	0f 87 01 02          	ja     0x3446
      3245:	b8 39 2c             	mov    $0x2c39,%ax
      3248:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      324d:	75 03                	jne    0x3252
      324f:	b8 99 2d             	mov    $0x2d99,%ax
      3252:	2b c2                	sub    %dx,%ax
      3254:	33 ff                	xor    %di,%di
      3256:	b9 00 01             	mov    $0x100,%cx
      3259:	66 c1 e0 10          	shl    $0x10,%eax
      325d:	b8 00 e8             	mov    $0xe800,%ax
      3260:	66 c1 c8 08          	ror    $0x8,%eax
      3264:	fc                   	cld
      3265:	66 ab                	stos   %eax,%es:(%di)
      3267:	66 2d 00 04 00 00    	sub    $0x400,%eax
      326d:	e2 f6                	loop   0x3265
      326f:	66 33 d2             	xor    %edx,%edx
      3272:	8b fa                	mov    %dx,%di
      3274:	8c c2                	mov    %es,%dx
      3276:	83 c2 40             	add    $0x40,%dx
      3279:	1e                   	push   %ds
      327a:	52                   	push   %dx
      327b:	89 16 1c 02          	mov    %dx,0x21c
      327f:	66 c1 e2 04          	shl    $0x4,%edx
      3283:	66 89 16 18 02       	mov    %edx,0x218
      3288:	b8 00 35             	mov    $0x3500,%ax
      328b:	1f                   	pop    %ds
      328c:	cd 21                	int    $0x21
      328e:	89 1d                	mov    %bx,(%di)
      3290:	8c 45 02             	mov    %es,0x2(%di)
      3293:	83 c7 04             	add    $0x4,%di
      3296:	fe c0                	inc    %al
      3298:	75 f2                	jne    0x328c
      329a:	66 33 d2             	xor    %edx,%edx
      329d:	8b fa                	mov    %dx,%di
      329f:	8c da                	mov    %ds,%dx
      32a1:	1f                   	pop    %ds
      32a2:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      32a7:	75 01                	jne    0x32aa
      32a9:	c3                   	ret
      32aa:	1e                   	push   %ds
      32ab:	66 ff 36 00 02       	pushl  0x200
      32b0:	8e da                	mov    %dx,%ds
      32b2:	66 8f 06 90 00       	popl   0x90
      32b7:	1f                   	pop    %ds
      32b8:	83 c2 40             	add    $0x40,%dx
      32bb:	8e c2                	mov    %dx,%es
      32bd:	66 c1 e2 04          	shl    $0x4,%edx
      32c1:	66 89 16 fe 00       	mov    %edx,0xfe
      32c6:	b9 10 00             	mov    $0x10,%cx
      32c9:	66 68 c1 30 08 00    	pushl  $0x830c1
      32cf:	66 58                	pop    %eax
      32d1:	66 33 db             	xor    %ebx,%ebx
      32d4:	b7 ee                	mov    $0xee,%bh
      32d6:	66 ab                	stos   %eax,%es:(%di)
      32d8:	26 66 89 1d          	mov    %ebx,%es:(%di)
      32dc:	83 c7 04             	add    $0x4,%di
      32df:	05 04 00             	add    $0x4,%ax
      32e2:	e2 f2                	loop   0x32d6
      32e4:	b9 f0 00             	mov    $0xf0,%cx
      32e7:	a1 06 02             	mov    0x206,%ax
      32ea:	05 40 00             	add    $0x40,%ax
      32ed:	66 ab                	stos   %eax,%es:(%di)
      32ef:	26 66 89 1d          	mov    %ebx,%es:(%di)
      32f3:	83 c7 04             	add    $0x4,%di
      32f6:	05 04 00             	add    $0x4,%ax
      32f9:	e2 f2                	loop   0x32ed
      32fb:	0f b6 3e 1e 02       	movzbw 0x21e,%di
      3300:	83 ff 08             	cmp    $0x8,%di
      3303:	74 12                	je     0x3317
      3305:	c1 e7 03             	shl    $0x3,%di
      3308:	b8 24 2f             	mov    $0x2f24,%ax
      330b:	b9 08 00             	mov    $0x8,%cx
      330e:	ab                   	stos   %ax,%es:(%di)
      330f:	83 c7 06             	add    $0x6,%di
      3312:	05 04 00             	add    $0x4,%ax
      3315:	e2 f7                	loop   0x330e
      3317:	b8 44 2f             	mov    $0x2f44,%ax
      331a:	0f b6 3e 1f 02       	movzbw 0x21f,%di
      331f:	c1 e7 03             	shl    $0x3,%di
      3322:	b1 08                	mov    $0x8,%cl
      3324:	ab                   	stos   %ax,%es:(%di)
      3325:	83 c7 06             	add    $0x6,%di
      3328:	05 04 00             	add    $0x4,%ax
      332b:	e2 f7                	loop   0x3324
      332d:	26 c7 06 98 01 eb 2d 	movw   $0x2deb,%es:0x198
      3334:	26 c7 06 08 01 7d 19 	movw   $0x197d,%es:0x108
      333b:	66 8b 0e c4 04       	mov    0x4c4,%ecx
      3340:	32 c9                	xor    %cl,%cl
      3342:	66 c1 e1 04          	shl    $0x4,%ecx
      3346:	66 89 0e c4 04       	mov    %ecx,0x4c4
      334b:	66 33 db             	xor    %ebx,%ebx
      334e:	8c c3                	mov    %es,%bx
      3350:	81 c3 80 00          	add    $0x80,%bx
      3354:	f7 db                	neg    %bx
      3356:	32 db                	xor    %bl,%bl
      3358:	f7 db                	neg    %bx
      335a:	89 1e ee 01          	mov    %bx,0x1ee
      335e:	66 c1 e3 04          	shl    $0x4,%ebx
      3362:	66 89 1e f0 01       	mov    %ebx,0x1f0
      3367:	66 03 1e e4 06       	add    0x6e4,%ebx
      336c:	66 81 c3 00 20 00 00 	add    $0x2000,%ebx
      3373:	66 2b d9             	sub    %ecx,%ebx
      3376:	76 18                	jbe    0x3390
      3378:	66 f7 db             	neg    %ebx
      337b:	66 03 1e e4 06       	add    0x6e4,%ebx
      3380:	66 81 fb 00 10 00 00 	cmp    $0x1000,%ebx
      3387:	0f 8c c0 00          	jl     0x344b
      338b:	66 89 1e e4 06       	mov    %ebx,0x6e4
      3390:	66 8b 1e f0 01       	mov    0x1f0,%ebx
      3395:	66 03 1e e4 06       	add    0x6e4,%ebx
      339a:	66 f7 db             	neg    %ebx
      339d:	81 e3 00 f0          	and    $0xf000,%bx
      33a1:	66 f7 db             	neg    %ebx
      33a4:	66 53                	push   %ebx
      33a6:	66 c1 eb 04          	shr    $0x4,%ebx
      33aa:	8e c3                	mov    %bx,%es
      33ac:	66 33 ff             	xor    %edi,%edi
      33af:	66 5b                	pop    %ebx
      33b1:	66 89 1e f0 04       	mov    %ebx,0x4f0
      33b6:	66 2b 1e c4 04       	sub    0x4c4,%ebx
      33bb:	66 f7 db             	neg    %ebx
      33be:	66 0f b6 06 e8 06    	movzbl 0x6e8,%eax
      33c4:	66 c1 e0 0c          	shl    $0xc,%eax
      33c8:	66 2b d8             	sub    %eax,%ebx
      33cb:	0f 86 69 00          	jbe    0x3438
      33cf:	66 81 fb 00 20 00 00 	cmp    $0x2000,%ebx
      33d6:	72 60                	jb     0x3438
      33d8:	66 29 06 c4 04       	sub    %eax,0x4c4
      33dd:	06                   	push   %es
      33de:	66 53                	push   %ebx
      33e0:	66 8b 1e c4 04       	mov    0x4c4,%ebx
      33e5:	66 c1 eb 04          	shr    $0x4,%ebx
      33e9:	2b 1e 7c 01          	sub    0x17c,%bx
      33ed:	b4 4a                	mov    $0x4a,%ah
      33ef:	8e 06 7c 01          	mov    0x17c,%es
      33f3:	cd 21                	int    $0x21
      33f5:	66 5b                	pop    %ebx
      33f7:	07                   	pop    %es
      33f8:	06                   	push   %es
      33f9:	66 33 c0             	xor    %eax,%eax
      33fc:	66 33 c9             	xor    %ecx,%ecx
      33ff:	66 c1 eb 02          	shr    $0x2,%ebx
      3403:	b9 00 40             	mov    $0x4000,%cx
      3406:	66 0b db             	or     %ebx,%ebx
      3409:	74 1b                	je     0x3426
      340b:	66 81 fb 00 40 00 00 	cmp    $0x4000,%ebx
      3412:	77 02                	ja     0x3416
      3414:	8b cb                	mov    %bx,%cx
      3416:	66 2b d9             	sub    %ecx,%ebx
      3419:	f3 66 ab             	rep stos %eax,%es:(%di)
      341c:	8c c1                	mov    %es,%cx
      341e:	81 c1 00 10          	add    $0x1000,%cx
      3422:	8e c1                	mov    %cx,%es
      3424:	eb dd                	jmp    0x3403
      3426:	58                   	pop    %ax
      3427:	8e c0                	mov    %ax,%es
      3429:	bf 00 10             	mov    $0x1000,%di
      342c:	05 00 02             	add    $0x200,%ax
      342f:	66 c1 e0 04          	shl    $0x4,%eax
      3433:	66 a3 c0 04          	mov    %eax,0x4c0
      3437:	c3                   	ret
      3438:	66 03 c3             	add    %ebx,%eax
      343b:	66 bb 00 20 00 00    	mov    $0x2000,%ebx
      3441:	66 2b c3             	sub    %ebx,%eax
      3444:	eb 92                	jmp    0x33d8
      3446:	ba c6 05             	mov    $0x5c6,%dx
      3449:	eb 03                	jmp    0x344e
      344b:	ba 62 05             	mov    $0x562,%dx
      344e:	16                   	push   %ss
      344f:	1f                   	pop    %ds
      3450:	b4 09                	mov    $0x9,%ah
      3452:	cd 21                	int    $0x21
      3454:	6a 01                	push   $0x1
      3456:	e9 32 d3             	jmp    0x78b
      3459:	ba 96 05             	mov    $0x596,%dx
      345c:	b0 00                	mov    $0x0,%al
      345e:	2e ff 16 14 14       	call   *%cs:0x1414
      3463:	16                   	push   %ss
      3464:	1f                   	pop    %ds
      3465:	b4 09                	mov    $0x9,%ah
      3467:	cd 21                	int    $0x21
      3469:	b0 01                	mov    $0x1,%al
      346b:	2e ff 16 16 14       	call   *%cs:0x1416
      3470:	6a 01                	push   $0x1
      3472:	e9 dd d2             	jmp    0x752
      3475:	66 8b 0e 48 01       	mov    0x148,%ecx
      347a:	66 81 e9 00 12 00 00 	sub    $0x1200,%ecx
      3481:	66 89 0e 48 01       	mov    %ecx,0x148
      3486:	81 e1 00 f0          	and    $0xf000,%cx
      348a:	66 89 0e d4 04       	mov    %ecx,0x4d4
      348f:	66 8b 16 ec 00       	mov    0xec,%edx
      3494:	66 8b ca             	mov    %edx,%ecx
      3497:	66 f7 d9             	neg    %ecx
      349a:	81 e1 00 f0          	and    $0xf000,%cx
      349e:	66 f7 d9             	neg    %ecx
      34a1:	66 89 0e d0 04       	mov    %ecx,0x4d0
      34a6:	66 81 c1 00 20 00 00 	add    $0x2000,%ecx
      34ad:	66 29 0e d8 04       	sub    %ecx,0x4d8
      34b2:	72 a5                	jb     0x3459
      34b4:	66 8b 0e 84 01       	mov    0x184,%ecx
      34b9:	66 3b ca             	cmp    %edx,%ecx
      34bc:	72 03                	jb     0x34c1
      34be:	66 8b ca             	mov    %edx,%ecx
      34c1:	8b 1e 82 01          	mov    0x182,%bx
      34c5:	66 8b 16 cc 04       	mov    0x4cc,%edx
      34ca:	0f b6 3e fc 04       	movzbw 0x4fc,%di
      34cf:	2e a1 14 10          	mov    %cs:0x1014,%ax
      34d3:	8e e0                	mov    %ax,%fs
      34d5:	8e d8                	mov    %ax,%ds
      34d7:	33 c0                	xor    %ax,%ax
      34d9:	8e c0                	mov    %ax,%es
      34db:	8e e8                	mov    %ax,%gs
      34dd:	b4 3f                	mov    $0x3f,%ah
      34df:	66 0e                	pushl  %cs
      34e1:	66 68 28 45 00 00    	pushl  $0x4528
      34e7:	1e                   	push   %ds
      34e8:	06                   	push   %es
      34e9:	0f a0                	push   %fs
      34eb:	0f a8                	push   %gs
      34ed:	66 50                	push   %eax
      34ef:	e9 b6 d3             	jmp    0x8a8
      34f2:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      34f7:	2e 8e 06 18 10       	mov    %cs:0x1018,%es
      34fc:	b4 3e                	mov    $0x3e,%ah
      34fe:	2e ff 16 12 16       	call   *%cs:0x1612
      3503:	26 67 8c 25 05 08 00 	addr32 mov %fs,%es:0x805
      350a:	00 
      350b:	66 8b 3e 48 01       	mov    0x148,%edi
      3510:	66 57                	push   %edi
      3512:	66 47                	inc    %edi
      3514:	66 89 3e 08 02       	mov    %edi,0x208
      3519:	66 81 ef 1d bf 00 00 	sub    $0xbf1d,%edi
      3520:	66 89 3e 0c 02       	mov    %edi,0x20c
      3525:	66 5f                	pop    %edi
      3527:	b8 0e 17             	mov    $0x170e,%ax
      352a:	66 b9 00 01 00 00    	mov    $0x100,%ecx
      3530:	67 f3 ab             	rep stos %ax,%es:(%edi)
      3533:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      3538:	0f 84 4d 00          	je     0x3589
      353c:	66 89 3e 14 02       	mov    %edi,0x214
      3541:	b9 00 01             	mov    $0x100,%cx
      3544:	66 8b 36 fe 00       	mov    0xfe,%esi
      3549:	64 67 66 8b 06       	mov    %fs:(%esi),%eax
      354e:	67 ab                	stos   %ax,%es:(%edi)
      3550:	33 c0                	xor    %ax,%ax
      3552:	67 66 ab             	stos   %eax,%es:(%edi)
      3555:	66 83 c7 02          	add    $0x2,%edi
      3559:	66 83 c6 08          	add    $0x8,%esi
      355d:	e2 ea                	loop   0x3549
      355f:	66 57                	push   %edi
      3561:	66 8b 3e 14 02       	mov    0x214,%edi
      3566:	66 b9 08 00 00 00    	mov    $0x8,%ecx
      356c:	66 0f b6 36 1e 02    	movzbl 0x21e,%esi
      3572:	67 66 8d 3c f7       	lea    (%edi,%esi,8),%edi
      3577:	b8 24 2f             	mov    $0x2f24,%ax
      357a:	67 ab                	stos   %ax,%es:(%edi)
      357c:	66 83 c7 06          	add    $0x6,%edi
      3580:	05 04 00             	add    $0x4,%ax
      3583:	e2 f5                	loop   0x357a
      3585:	66 5f                	pop    %edi
      3587:	eb 25                	jmp    0x35ae
      3589:	66 89 3e 14 02       	mov    %edi,0x214
      358e:	b8 04 02             	mov    $0x204,%ax
      3591:	66 33 c9             	xor    %ecx,%ecx
      3594:	33 db                	xor    %bx,%bx
      3596:	cd 31                	int    $0x31
      3598:	26 67 66 89 17       	mov    %edx,%es:(%edi)
      359d:	66 83 c7 04          	add    $0x4,%edi
      35a1:	26 67 66 89 0f       	mov    %ecx,%es:(%edi)
      35a6:	66 83 c7 04          	add    $0x4,%edi
      35aa:	fe c3                	inc    %bl
      35ac:	75 e8                	jne    0x3596
      35ae:	66 89 3e 10 02       	mov    %edi,0x210
      35b3:	2e 66 0f b7 06 16 10 	movzwl %cs:0x1016,%eax
      35ba:	66 8b 1e 08 02       	mov    0x208,%ebx
      35bf:	66 4b                	dec    %ebx
      35c1:	b9 00 01             	mov    $0x100,%cx
      35c4:	26 67 66 89 1f       	mov    %ebx,%es:(%edi)
      35c9:	66 83 c3 02          	add    $0x2,%ebx
      35cd:	66 83 c7 04          	add    $0x4,%edi
      35d1:	26 67 66 89 07       	mov    %eax,%es:(%edi)
      35d6:	66 83 c7 04          	add    $0x4,%edi
      35da:	e2 e8                	loop   0x35c4
      35dc:	1e                   	push   %ds
      35dd:	ff 36 1e 02          	push   0x21e
      35e1:	67 66 0f b6 34 24    	movzbl (%esp),%esi
      35e7:	66 8b 3e 10 02       	mov    0x210,%edi
      35ec:	66 57                	push   %edi
      35ee:	06                   	push   %es
      35ef:	1f                   	pop    %ds
      35f0:	67 66 8d 34 f7       	lea    (%edi,%esi,8),%esi
      35f5:	67 66 8d 7f 40       	lea    0x40(%edi),%edi
      35fa:	66 b9 10 00 00 00    	mov    $0x10,%ecx
      3600:	67 f3 66 a5          	rep movsl %ds:(%esi),%es:(%edi)
      3604:	66 5f                	pop    %edi
      3606:	67 66 0f b6 74 24 01 	movzbl 0x1(%esp),%esi
      360d:	67 66 8d 34 f7       	lea    (%edi,%esi,8),%esi
      3612:	67 66 8d bf 80 03 00 	lea    0x380(%edi),%edi
      3619:	00 
      361a:	66 b9 10 00 00 00    	mov    $0x10,%ecx
      3620:	67 f3 66 a5          	rep movsl %ds:(%esi),%es:(%edi)
      3624:	58                   	pop    %ax
      3625:	1f                   	pop    %ds
      3626:	80 3e fc 04 c0       	cmpb   $0xc0,0x4fc
      362b:	2e 8e 1e 18 10       	mov    %cs:0x1018,%ds
      3630:	2e 8e 26 1c 10       	mov    %cs:0x101c,%fs
      3635:	2e 8e 2e 1e 10       	mov    %cs:0x101e,%gs
      363a:	0f 84 1e 00          	je     0x365c
      363e:	58                   	pop    %ax
      363f:	2e 66 ff 36 1a 10    	pushl  %cs:0x101a
      3645:	36 66 ff 36 48 01    	pushl  %ss:0x148
      364b:	68 00 32             	push   $0x3200
      364e:	9d                   	popf
      364f:	2e 66 ff 36 16 10    	pushl  %cs:0x1016
      3655:	66 68 a8 09 00 00    	pushl  $0x9a8
      365b:	50                   	push   %ax
      365c:	c3                   	ret
      365d:	2e c7 06 16 14 73 15 	movw   $0x1573,%cs:0x1416
      3664:	2e c7 06 14 14 5c 14 	movw   $0x145c,%cs:0x1414
      366b:	b8 0a de             	mov    $0xde0a,%ax
      366e:	cd 67                	int    $0x67
      3670:	8a f9                	mov    %cl,%bh
      3672:	89 1e 1e 02          	mov    %bx,0x21e
      3676:	be f5 2e             	mov    $0x2ef5,%si
      3679:	b9 08 00             	mov    $0x8,%cx
      367c:	2e 88 1c             	mov    %bl,%cs:(%si)
      367f:	2e 88 7c 18          	mov    %bh,%cs:0x18(%si)
      3683:	83 c6 03             	add    $0x3,%si
      3686:	81 c3 01 01          	add    $0x101,%bx
      368a:	e2 f0                	loop   0x367c
      368c:	e8 ca e7             	call   0x1e59
      368f:	e8 7a fb             	call   0x320c
      3692:	be 68 00             	mov    $0x68,%si
      3695:	b8 01 de             	mov    $0xde01,%ax
      3698:	cd 67                	int    $0x67
      369a:	0a e4                	or     %ah,%ah
      369c:	0f 85 52 00          	jne    0x36f2
      36a0:	2e 66 89 1e 22 10    	mov    %ebx,%cs:0x1022
      36a6:	66 33 f6             	xor    %esi,%esi
      36a9:	be 00 00             	mov    $0x0,%si
      36ac:	66 c1 e6 04          	shl    $0x4,%esi
      36b0:	66 01 36 e8 05       	add    %esi,0x5e8
      36b5:	66 01 36 ec 05       	add    %esi,0x5ec
      36ba:	66 81 c6 e4 05 00 00 	add    $0x5e4,%esi
      36c1:	66 89 36 fa 05       	mov    %esi,0x5fa
      36c6:	e8 48 f3             	call   0x2a11
      36c9:	2e ff 16 16 14       	call   *%cs:0x1416
      36ce:	16                   	push   %ss
      36cf:	1f                   	pop    %ds
      36d0:	e8 a2 fd             	call   0x3475
      36d3:	66 cb                	lretl
      36d5:	46                   	inc    %si
      36d6:	61                   	popa
      36d7:	74 61                	je     0x373a
      36d9:	6c                   	insb   (%dx),%es:(%di)
      36da:	20 65 72             	and    %ah,0x72(%di)
      36dd:	72 6f                	jb     0x374e
      36df:	72 20                	jb     0x3701
      36e1:	6d                   	insw   (%dx),%es:(%di)
      36e2:	61                   	popa
      36e3:	6b 69 6e 67          	imul   $0x67,0x6e(%bx,%di),%bp
      36e7:	20 56 43             	and    %dl,0x43(%bp)
      36ea:	50                   	push   %ax
      36eb:	49                   	dec    %cx
      36ec:	20 63 61             	and    %ah,0x61(%bp,%di)
      36ef:	6c                   	insb   (%dx),%es:(%di)
      36f0:	6c                   	insb   (%dx),%es:(%di)
      36f1:	24 b4                	and    $0xb4,%al
      36f3:	09 ba 0b 47          	or     %di,0x470b(%bp,%si)
      36f7:	0e                   	push   %cs
      36f8:	1f                   	pop    %ds
      36f9:	cd 21                	int    $0x21
      36fb:	b8 01 4c             	mov    $0x4c01,%ax
      36fe:	cd 21                	int    $0x21
      3700:	66 57                	push   %edi
      3702:	66 52                	push   %edx
      3704:	5a                   	pop    %dx
      3705:	2e c7 06 14 14 94 14 	movw   $0x1494,%cs:0x1414
      370c:	2e c7 06 16 14 ac 15 	movw   $0x15ac,%cs:0x1416
      3713:	2e c6 06 d1 3c c3    	movb   $0xc3,%cs:0x3cd1
      3719:	2e c6 06 ff 30 e9    	movb   $0xe9,%cs:0x30ff
      371f:	2e c7 06 00 31 e2 fe 	movw   $0xfee2,%cs:0x3100
      3726:	e8 e3 fa             	call   0x320c
      3729:	5a                   	pop    %dx
      372a:	66 0f b7 06 1c 02    	movzwl 0x21c,%eax
      3730:	05 40 00             	add    $0x40,%ax
      3733:	50                   	push   %ax
      3734:	03 c2                	add    %dx,%ax
      3736:	05 20 00             	add    $0x20,%ax
      3739:	a3 ee 01             	mov    %ax,0x1ee
      373c:	66 c1 e0 04          	shl    $0x4,%eax
      3740:	66 a3 f0 01          	mov    %eax,0x1f0
      3744:	66 03 06 e4 06       	add    0x6e4,%eax
      3749:	66 0f b7 0e c4 04    	movzwl 0x4c4,%ecx
      374f:	66 c1 e1 04          	shl    $0x4,%ecx
      3753:	66 89 0e c4 04       	mov    %ecx,0x4c4
      3758:	66 2b c1             	sub    %ecx,%eax
      375b:	76 16                	jbe    0x3773
      375d:	66 f7 d8             	neg    %eax
      3760:	66 03 06 e4 06       	add    0x6e4,%eax
      3765:	66 3d 00 10 00 00    	cmp    $0x1000,%eax
      376b:	0f 8c dc fc          	jl     0x344b
      376f:	66 a3 e4 06          	mov    %eax,0x6e4
      3773:	66 8b 1e e4 06       	mov    0x6e4,%ebx
      3778:	66 c1 eb 04          	shr    $0x4,%ebx
      377c:	03 1e ee 01          	add    0x1ee,%bx
      3780:	66 c1 e3 04          	shl    $0x4,%ebx
      3784:	66 89 1e c0 04       	mov    %ebx,0x4c0
      3789:	66 89 1e c4 04       	mov    %ebx,0x4c4
      378e:	66 c1 eb 04          	shr    $0x4,%ebx
      3792:	2b 1e 7c 01          	sub    0x17c,%bx
      3796:	b4 4a                	mov    $0x4a,%ah
      3798:	8e 06 7c 01          	mov    0x17c,%es
      379c:	cd 21                	int    $0x21
      379e:	0f 82 a9 fc          	jb     0x344b
      37a2:	07                   	pop    %es
      37a3:	b8 01 00             	mov    $0x1,%ax
      37a6:	67 ff 1c 24          	lcall  *(%esp)
      37aa:	0f 82 a0 02          	jb     0x3a4e
      37ae:	83 c4 04             	add    $0x4,%sp
      37b1:	bc d4 00             	mov    $0xd4,%sp
      37b4:	8c cb                	mov    %cs,%bx
      37b6:	b8 0a 00             	mov    $0xa,%ax
      37b9:	cd 31                	int    $0x31
      37bb:	8e d8                	mov    %ax,%ds
      37bd:	8c 0e 10 10          	mov    %cs,0x1010
      37c1:	8c 16 12 10          	mov    %ss,0x1012
      37c5:	8c 06 1c 10          	mov    %es,0x101c
      37c9:	26 a1 2c 00          	mov    %es:0x2c,%ax
      37cd:	a3 1e 10             	mov    %ax,0x101e
      37d0:	c7 06 12 16 8d 16    	movw   $0x168d,0x1612
      37d6:	33 c0                	xor    %ax,%ax
      37d8:	b9 04 00             	mov    $0x4,%cx
      37db:	cd 31                	int    $0x31
      37dd:	0f 82 7d 02          	jb     0x3a5e
      37e1:	a3 14 10             	mov    %ax,0x1014
      37e4:	8b d8                	mov    %ax,%bx
      37e6:	b8 03 00             	mov    $0x3,%ax
      37e9:	cd 31                	int    $0x31
      37eb:	03 d8                	add    %ax,%bx
      37ed:	89 1e 1a 10          	mov    %bx,0x101a
      37f1:	03 d8                	add    %ax,%bx
      37f3:	89 1e 16 10          	mov    %bx,0x1016
      37f7:	03 d8                	add    %ax,%bx
      37f9:	89 1e 18 10          	mov    %bx,0x1018
      37fd:	66 0f 02 0e 10 10    	lar    0x1010,%ecx
      3803:	66 c1 e9 08          	shr    $0x8,%ecx
      3807:	b8 09 00             	mov    $0x9,%ax
      380a:	81 c9 9b c0          	or     $0xc09b,%cx
      380e:	8b 1e 16 10          	mov    0x1016,%bx
      3812:	cd 31                	int    $0x31
      3814:	0f 82 46 02          	jb     0x3a5e
      3818:	80 e1 f7             	and    $0xf7,%cl
      381b:	8b 1e 14 10          	mov    0x1014,%bx
      381f:	cd 31                	int    $0x31
      3821:	0f 82 39 02          	jb     0x3a5e
      3825:	80 c9 04             	or     $0x4,%cl
      3828:	8b 1e 1a 10          	mov    0x101a,%bx
      382c:	cd 31                	int    $0x31
      382e:	0f 82 2c 02          	jb     0x3a5e
      3832:	8b 1e 18 10          	mov    0x1018,%bx
      3836:	cd 31                	int    $0x31
      3838:	0f 82 22 02          	jb     0x3a5e
      383c:	b8 08 00             	mov    $0x8,%ax
      383f:	ba ff ff             	mov    $0xffff,%dx
      3842:	8b ca                	mov    %dx,%cx
      3844:	0f 82 16 02          	jb     0x3a5e
      3848:	8b 1e 14 10          	mov    0x1014,%bx
      384c:	cd 31                	int    $0x31
      384e:	0f 82 0c 02          	jb     0x3a5e
      3852:	8b 1e 18 10          	mov    0x1018,%bx
      3856:	ba 7f 00             	mov    $0x7f,%dx
      3859:	33 c9                	xor    %cx,%cx
      385b:	cd 31                	int    $0x31
      385d:	0f 82 fd 01          	jb     0x3a5e
      3861:	8c db                	mov    %ds,%bx
      3863:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      3868:	b8 01 00             	mov    $0x1,%ax
      386b:	cd 31                	int    $0x31
      386d:	b8 01 05             	mov    $0x501,%ax
      3870:	66 8b 16 ec 00       	mov    0xec,%edx
      3875:	66 81 c2 ff 4f 00 00 	add    $0x4fff,%edx
      387c:	81 e2 00 f0          	and    $0xf000,%dx
      3880:	66 52                	push   %edx
      3882:	59                   	pop    %cx
      3883:	5b                   	pop    %bx
      3884:	cd 31                	int    $0x31
      3886:	0f 82 d4 01          	jb     0x3a5e
      388a:	89 36 02 06          	mov    %si,0x602
      388e:	89 3e 00 06          	mov    %di,0x600
      3892:	53                   	push   %bx
      3893:	51                   	push   %cx
      3894:	66 58                	pop    %eax
      3896:	66 50                	push   %eax
      3898:	66 03 d0             	add    %eax,%edx
      389b:	66 05 ff 0f 00 00    	add    $0xfff,%eax
      38a1:	25 00 f0             	and    $0xf000,%ax
      38a4:	81 e2 00 f0          	and    $0xf000,%dx
      38a8:	66 2b d0             	sub    %eax,%edx
      38ab:	66 8b d8             	mov    %eax,%ebx
      38ae:	66 89 16 48 01       	mov    %edx,0x148
      38b3:	66 89 16 d8 04       	mov    %edx,0x4d8
      38b8:	66 58                	pop    %eax
      38ba:	66 2b d8             	sub    %eax,%ebx
      38bd:	66 89 1e 08 06       	mov    %ebx,0x608
      38c2:	e8 af f8             	call   0x3174
      38c5:	2e 8b 1e 16 10       	mov    %cs:0x1016,%bx
      38ca:	b8 08 00             	mov    $0x8,%ax
      38cd:	b9 ff ff             	mov    $0xffff,%cx
      38d0:	8b d1                	mov    %cx,%dx
      38d2:	cd 31                	int    $0x31
      38d4:	0f 82 86 01          	jb     0x3a5e
      38d8:	b9 19 00             	mov    $0x19,%cx
      38db:	be 10 06             	mov    $0x610,%si
      38de:	8c 0c                	mov    %cs,(%si)
      38e0:	8d 74 06             	lea    0x6(%si),%si
      38e3:	e2 f9                	loop   0x38de
      38e5:	be 6c 06             	mov    $0x66c,%si
      38e8:	8b 3e 04 02          	mov    0x204,%di
      38ec:	81 ef 6f 00          	sub    $0x6f,%di
      38f0:	c1 e7 04             	shl    $0x4,%di
      38f3:	83 c7 40             	add    $0x40,%di
      38f6:	b1 06                	mov    $0x6,%cl
      38f8:	89 3c                	mov    %di,(%si)
      38fa:	8d 7d 04             	lea    0x4(%di),%di
      38fd:	8d 74 06             	lea    0x6(%si),%si
      3900:	e2 f6                	loop   0x38f8
      3902:	e8 91 01             	call   0x3a96
      3905:	66 33 ff             	xor    %edi,%edi
      3908:	b8 05 03             	mov    $0x305,%ax
      390b:	cd 31                	int    $0x31
      390d:	0f 82 4d 01          	jb     0x3a5e
      3911:	a3 20 00             	mov    %ax,0x20
      3914:	0b c0                	or     %ax,%ax
      3916:	75 0e                	jne    0x3926
      3918:	b9 fc 14             	mov    $0x14fc,%cx
      391b:	66 33 ff             	xor    %edi,%edi
      391e:	bf fb 14             	mov    $0x14fb,%di
      3921:	8c ce                	mov    %cs,%si
      3923:	bb 6f 00             	mov    $0x6f,%bx
      3926:	66 89 3e 10 00       	mov    %edi,0x10
      392b:	89 36 14 00          	mov    %si,0x14
      392f:	53                   	push   %bx
      3930:	51                   	push   %cx
      3931:	66 8f 06 18 00       	popl   0x18
      3936:	a1 ee 01             	mov    0x1ee,%ax
      3939:	2d 00 00             	sub    $0x0,%ax
      393c:	c1 e0 04             	shl    $0x4,%ax
      393f:	a3 28 00             	mov    %ax,0x28
      3942:	a3 2a 00             	mov    %ax,0x2a
      3945:	b8 06 03             	mov    $0x306,%ax
      3948:	cd 31                	int    $0x31
      394a:	0f 82 10 01          	jb     0x3a5e
      394e:	66 89 3e 30 00       	mov    %edi,0x30
      3953:	89 36 34 00          	mov    %si,0x34
      3957:	53                   	push   %bx
      3958:	51                   	push   %cx
      3959:	66 8f 06 38 00       	popl   0x38
      395e:	e8 14 fb             	call   0x3475
      3961:	1e                   	push   %ds
      3962:	06                   	push   %es
      3963:	16                   	push   %ss
      3964:	1f                   	pop    %ds
      3965:	e8 9f 01             	call   0x3b07
      3968:	07                   	pop    %es
      3969:	1f                   	pop    %ds
      396a:	36 66 8b 0e cc 04    	mov    %ss:0x4cc,%ecx
      3970:	66 f7 d9             	neg    %ecx
      3973:	67 66 89 0d 07 08 00 	addr32 mov %ecx,0x807
      397a:	00 
      397b:	36 66 8b 0e 48 01    	mov    %ss:0x148,%ecx
      3981:	66 81 c1 ff 0f 00 00 	add    $0xfff,%ecx
      3988:	81 e1 00 f0          	and    $0xf000,%cx
      398c:	36 66 89 0e d0 04    	mov    %ecx,%ss:0x4d0
      3992:	2e a1 16 10          	mov    %cs:0x1016,%ax
      3996:	66 b9 08 00 00 00    	mov    $0x8,%ecx
      399c:	67 89 04 cd 30 bf 00 	mov    %ax,0xbf30(,%ecx,8)
      39a3:	00 
      39a4:	e2 f6                	loop   0x399c
      39a6:	33 c9                	xor    %cx,%cx
      39a8:	ba ff 00             	mov    $0xff,%dx
      39ab:	2e 8b 1e 1a 10       	mov    %cs:0x101a,%bx
      39b0:	b8 08 00             	mov    $0x8,%ax
      39b3:	cd 31                	int    $0x31
      39b5:	0f 82 a5 00          	jb     0x3a5e
      39b9:	b8 00 04             	mov    $0x400,%ax
      39bc:	cd 31                	int    $0x31
      39be:	86 f2                	xchg   %dh,%dl
      39c0:	36 89 16 1e 02       	mov    %dx,%ss:0x21e
      39c5:	b8 00 06             	mov    $0x600,%ax
      39c8:	36 66 8b 0e 48 01    	mov    %ss:0x148,%ecx
      39ce:	36 66 03 0e cc 04    	add    %ss:0x4cc,%ecx
      39d4:	66 0f a4 cb 10       	shld   $0x10,%ecx,%ebx
      39d9:	33 f6                	xor    %si,%si
      39db:	bf 00 12             	mov    $0x1200,%di
      39de:	cd 31                	int    $0x31
      39e0:	0f 82 7a 00          	jb     0x3a5e
      39e4:	66 b9 04 bf 00 00    	mov    $0xbf04,%ecx
      39ea:	36 66 03 0e cc 04    	add    %ss:0x4cc,%ecx
      39f0:	bf 40 00             	mov    $0x40,%di
      39f3:	66 0f a4 cb 10       	shld   $0x10,%ecx,%ebx
      39f8:	cd 31                	int    $0x31
      39fa:	0f 82 60 00          	jb     0x3a5e
      39fe:	66 b9 7c bf 00 00    	mov    $0xbf7c,%ecx
      3a04:	36 66 03 0e cc 04    	add    %ss:0x4cc,%ecx
      3a0a:	bf 0e 00             	mov    $0xe,%di
      3a0d:	66 0f a4 cb 10       	shld   $0x10,%ecx,%ebx
      3a12:	cd 31                	int    $0x31
      3a14:	0f 82 46 00          	jb     0x3a5e
      3a18:	36 66 8b 16 48 01    	mov    %ss:0x148,%edx
      3a1e:	2e 8e 16 1a 10       	mov    %cs:0x101a,%ss
      3a23:	66 8b e2             	mov    %edx,%esp
      3a26:	2e a1 12 10          	mov    %cs:0x1012,%ax
      3a2a:	67 a3 82 bf 00 00    	addr32 mov %ax,0xbf82
      3a30:	67 66 8c 0d 88 bf 00 	addr32 data32 mov %cs,0xbf88
      3a37:	00 
      3a38:	67 66 8c 0d 80 bf 00 	addr32 data32 mov %cs,0xbf80
      3a3f:	00 
      3a40:	2e 66 ff 36 16 10    	pushl  %cs:0x1016
      3a46:	66 68 a8 09 00 00    	pushl  $0x9a8
      3a4c:	66 cb                	lretl
      3a4e:	ba a2 06             	mov    $0x6a2,%dx
      3a51:	b4 09                	mov    $0x9,%ah
      3a53:	68 00 00             	push   $0x0
      3a56:	1f                   	pop    %ds
      3a57:	cd 21                	int    $0x21
      3a59:	b8 01 4c             	mov    $0x4c01,%ax
      3a5c:	cd 21                	int    $0x21
      3a5e:	16                   	push   %ss
      3a5f:	07                   	pop    %es
      3a60:	b4 09                	mov    $0x9,%ah
      3a62:	ba c6 06             	mov    $0x6c6,%dx
      3a65:	66 33 c9             	xor    %ecx,%ecx
      3a68:	66 51                	push   %ecx
      3a6a:	66 51                	push   %ecx
      3a6c:	66 51                	push   %ecx
      3a6e:	68 00 00             	push   $0x0
      3a71:	68 00 00             	push   $0x0
      3a74:	9c                   	pushf
      3a75:	66 60                	pushal
      3a77:	bb 21 00             	mov    $0x21,%bx
      3a7a:	b8 00 03             	mov    $0x300,%ax
      3a7d:	66 8b fc             	mov    %esp,%edi
      3a80:	cd 31                	int    $0x31
      3a82:	b8 01 4c             	mov    $0x4c01,%ax
      3a85:	cd 21                	int    $0x21
      3a87:	66 60                	pushal
      3a89:	1e                   	push   %ds
      3a8a:	2e 8e 1e 12 10       	mov    %cs:0x1012,%ds
      3a8f:	e8 19 00             	call   0x3aab
      3a92:	1f                   	pop    %ds
      3a93:	66 61                	popal
      3a95:	c3                   	ret
      3a96:	be 0c 06             	mov    $0x60c,%si
      3a99:	b3 00                	mov    $0x0,%bl
      3a9b:	b8 02 02             	mov    $0x202,%ax
      3a9e:	e8 51 00             	call   0x3af2
      3aa1:	83 c6 06             	add    $0x6,%si
      3aa4:	fe c3                	inc    %bl
      3aa6:	80 fb 0f             	cmp    $0xf,%bl
      3aa9:	76 f0                	jbe    0x3a9b
      3aab:	be 6c 06             	mov    $0x66c,%si
      3aae:	b4 02                	mov    $0x2,%ah
      3ab0:	2e 8b 1e 16 10       	mov    %cs:0x1016,%bx
      3ab5:	66 0f 03 db          	lsl    %ebx,%ebx
      3ab9:	66 d1 d3             	rcl    $1,%ebx
      3abc:	0f 83 2d 00          	jae    0x3aed
      3ac0:	b3 10                	mov    $0x10,%bl
      3ac2:	b0 04                	mov    $0x4,%al
      3ac4:	e8 2b 00             	call   0x3af2
      3ac7:	83 c6 06             	add    $0x6,%si
      3aca:	fe c3                	inc    %bl
      3acc:	80 fb 15             	cmp    $0x15,%bl
      3acf:	76 f1                	jbe    0x3ac2
      3ad1:	b3 21                	mov    $0x21,%bl
      3ad3:	b0 04                	mov    $0x4,%al
      3ad5:	e8 1a 00             	call   0x3af2
      3ad8:	83 c6 06             	add    $0x6,%si
      3adb:	b3 23                	mov    $0x23,%bl
      3add:	b0 04                	mov    $0x4,%al
      3adf:	e8 10 00             	call   0x3af2
      3ae2:	83 c6 06             	add    $0x6,%si
      3ae5:	b3 33                	mov    $0x33,%bl
      3ae7:	b0 04                	mov    $0x4,%al
      3ae9:	e8 06 00             	call   0x3af2
      3aec:	c3                   	ret
      3aed:	8d 74 24             	lea    0x24(%si),%si
      3af0:	eb df                	jmp    0x3ad1
      3af2:	cd 31                	int    $0x31
      3af4:	0f 82 66 ff          	jb     0x3a5e
      3af8:	66 87 14             	xchg   %edx,(%si)
      3afb:	87 4c 04             	xchg   %cx,0x4(%si)
      3afe:	fe c0                	inc    %al
