* File setmas.f generated by GRACE Ver. 2.00(35)        1996/03/24/15:33
* 
*          Fortran source code generator
*     (c)copyright 1990-1996 Minami-Tateya Group, Japan
*-----------------------------------------------------------------------
************************************************************************
      subroutine sm_koralw(ibackgr,xpar,npar,sin2w,gpicob,amafin)
************************************************************************
* Overwrites GRACE initialisation according to KORALW needs
* ibackgr = 0  - doubly resonant W-pairs
* ibackgr = 1  - complete 4fermion process
************************************************************************
      implicit DOUBLE PRECISION(a-h,o-z)
*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      dimension  xpar ( *),npar ( *)
      dimension  amafin(20)
*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      include 'incl1.f'
      include 'inclk.f'
      common /chcntl/ jwidth
      character*80 BXOPE,BXTXT,BXCLO,BXTXI,BXTXF
      data init /0/
      save init
*-----------------------------------------------------------------------
* constants
      pi    = acos(- 1.0d0 )
      pi2   = pi * pi
      rad   = pi / 180.0d0
      gevpb = 0.38937966d9
      alpha = 1.0d0/128.07d0
      alpha0= 1.0d0/137.0359895d0
*KEK  alphas= 0.123d0
      alphas= 0.12d0

************* KORALW stuff ******************
      ALFWIN  = XPAR(3)
      ALPHAW  = 1D0/ ALFWIN
      XAMAZ   = XPAR(4)
      GAMMZ   = XPAR(5)
      XAMAW   = XPAR(6)
      GAMMW   = XPAR(7)
       xamh   =xpar(11)
       xagh   =xpar(12)
! missing redefinition of alphas ! m.s. 6/19/98
      alphas  =xpar(13)

! missing redefinition of alphaw ! m.s. 12/5/97
      alpha = alphaw  ! m.s. 12/5/97
************* end KORALW stuff ******************
*-----------------------------------------------------------------------
      call selgrf( ibackgr )

      jgraph = 0
*-----------------------------------------------------------------------
* mass
*
         amw = 80.23D0
         amz = 91.1888D0
***** From KORAL (begin) *****
         amw = xamaw
         amz = xamaz
***** From KORAL (e n d) *****
         ama = 0.0D0
         amg = 0.0D0
         amh = 10000.0D0
***** From KORAL (begin) *****
         amh = xamh
***** From KORAL (e n d) *****
         amx = AMW
         amy = AMZ
        amne = 0.0D0
        amnm = 0.0D0
        amnt = 0.0D0
        amel = 0.51099906D-3
        ammu = 105.658389D-3
        amta = 1.7771D0

***** From KORAL (begin) *****
      AMNE = AMAFIN(12)
      AMNM = AMAFIN(14)
      AMNT = AMAFIN(16)
      AMEL = AMAFIN(11)
      AMMU = AMAFIN(13)
      AMTA = AMAFIN(15)
***** From KORAL (e n d) *****

        amuq = 5.0D-3
        amcq = 1.3
        amtq = 174.0D0
        amdq = 10.0D-3
        amsq = 200.0D-3
        ambq = 4.3D0
***** From KORAL (begin) *****
      AMUQ = AMAFIN(2)
      AMCQ = AMAFIN(4)
* 7/15/98 ms      AMTQ = 174.0D0
      AMTQ = AMAFIN(6)
      AMDQ = AMAFIN(1)
      AMSQ = AMAFIN(3)
      AMBQ = AMAFIN(5)
***** From KORAL (e n d) *****
        amcp = AMW
        amcm = AMW
        amcz = AMZ
        amca = AMA
        amcg = AMG

* set quark mass = 1.d-5
*
      if( jqmass .eq. 0 )then
*
        write(6,*) 'WARNING: sm_koralw=> quark masses set to 1d-5'
            amuq = 1.0d-5
            amcq = 1.0d-5
            amtq = 1.0d-5
            amdq = 1.0d-5
            amsq = 1.0d-5
            ambq = 1.0d-5
      endif
*
* width

         agw = 2.03367033062746D0
         agz = 2.4974D0
         agh = 0.0D0
***** From KORAL (begin) *****
      AGW  = GAMMW
      AGZ  = GAMMZ
      agh  = xagh
***** From KORAL (e n d) *****

         agx = AGW
         agy = AGZ
        agcq = 0.0D0
        agtq = 0.0D0
        agsq = 0.0D0
        agbq = 0.0D0
        agcp = AGW
        agcm = AGW
        agcz = AGZ

* Gauge parametes (default is unitary gauge)
      igauab = 0
      igauwb = 0
      igauzb = 0
      igaugb = 0
      agauge(igauab) = 1.0d0
      agauge(igauwb) = 1.0d0
      agauge(igauzb) = 1.0d0
      agauge(igaugb) = 1.0d0
      agauge(igau00) = 1.0d0

* Spin average
      aspin = 1.0d0

*     1: initial electron mass=amel
      jhs(1) = 0
      jhe(1) = lextrn - 1
      aspin = aspin/dble(jhe(1) - jhs(1)+1)

*     2: initial positron mass=amel
      jhs(2) = 0
      jhe(2) = lextrn - 1
      aspin = aspin/dble(jhe(2) - jhs(2)+1)

*     3: final nu-e mass=amne
      jhs(3) = 0
      jhe(3) = lextrn - 1

*     4: final positron mass=amel
      jhs(4) = 0
      jhe(4) = lextrn - 1

*     5: final electron mass=amel
      jhs(5) = 0
      jhe(5) = lextrn - 1

*     6: final nu-e-bar mass=amne
      jhs(6) = 0
      jhe(6) = lextrn - 1

* Flag of cyclic polarization
*     QED vertex with on-shell fermions.
!      jtgamm = 0
*     Anomalous coupling for 3-vector-boson.
      jano3v = 0
*     Coulomb correction.
*n    jcolmb = 0
*     QCD correction.
*n    jqcdcr = 0
*     Private flag (Internal Higgs)
      jhiggs = 1
*     Private flag (Internal Gluon)
      jgluon = 0

*     Private flag (Decay)
*n    jdecay = 0
*     Private flag (Hadronization)
*n    jhadrn = 1

*     Running width (0) or fixed width(1) in CHANEL
      jwidth = 0

*     Coulomb correction
      colmbf = 1.0d0
      if (init.eq.0) then
       init=1
       BXOPE =  '(//1X,15(5H*****)    )' 
       BXTXT =  '(1X,1H*,                  A48,25X,    1H*)'
       BXTXI =  '(1X,1H*,                  A48,I2,23X, 1H*)'
       BXTXF =  '(1X,1H*,                A48,F7.5,18X, 1H*)'
       BXCLO =  '(1X,15(5H*****)/   )' 
       NOUT=6
       WRITE(NOUT,BXOPE)
       WRITE(NOUT,BXTXT) 'Grace 2.0 initialization routine sm_koralw:'

       WRITE(NOUT,BXTXI) 'Higgs switch                 jhiggs =    '
     $                  ,jhiggs
       WRITE(NOUT,BXTXI) 'gluon switch                 jgluon =    '
     $                  ,jgluon
       WRITE(NOUT,BXTXF) 'gluon as intermediate boson: alpha_s=    '
     $                  ,alphas
       WRITE(NOUT,BXTXT) 'warning from sm_koralw:                  '
       WRITE(NOUT,BXTXT) 'please check consistency of the sin2w:   '
       WRITE(NOUT,BXTXT) 'as defined in KORALW vs required by GRACE'
       WRITE(NOUT,BXTXT) 'consult manuals of the two packages      '
       WRITE(NOUT,BXCLO)
! not needed to prt. write(*,*) 'Running width switch jwidth =',jwidth 
      endif
      return
      end


