
#include "state_go.hpp"
#include <cassert>
#include <queue>

#define FLAG_B    0x80000000
#define FLAG_W    0x40000000
#define FLAG_BOTH (FLAG_B | FLAG_W)
#define NO_FLAG   0x3fffffff

StateGo::StateGo(int size,float komi) : _size(size),_komi(komi)
#ifdef JAPANESE
        ,captured_b(0),captured_w(0)
#endif
{
    Stones = new Player*[_size];
    for(int i=0;i<_size;i++)
        Stones[i] = new Player[_size];

    Blocks = new int**[_size];
    for(int i=0;i<_size;i++)
        Blocks[i] = new int*[_size];
    
    for(int i=0;i<_size;i++)
        for(int j=0;j<_size;j++){
            Stones[i][j]=Empty;
            Blocks[i][j]=NULL;
        }
    pass=0;
    ko_flag=false;
    ko_unique=false;
    turn=Black;
}

StateGo *StateGo::copy()
{
    StateGo *p= new StateGo(_size);
    for(int i=0;i<_size;i++)
        for(int j=0;j<_size;j++){
            p->Stones[i][j]=Stones[i][j];
            p->Blocks[i][j]=Blocks[i][j];
        }
    for(int i=0;i<_size;i++)
        for(int j=0;j<_size;j++)
            if(p->Stones[i][j]!=Empty && p->Blocks[i][j]==Blocks[i][j]){
                p->Blocks[i][j]=new int;
                *(p->Blocks[i][j])=*Blocks[i][j];
                p->update_block(Blocks[i][j],p->Blocks[i][j],i,j);
            }
    p->koi=koi;
    p->koj=koj;
    p->ko_flag=ko_flag;
    p->ko_unique=ko_unique;
    p->pass=pass;
    p->turn=turn;
    return p;
}

StateGo::~StateGo()
{
    for(int i=0;i<_size;i++){
      for(int j=0;j<_size;j++)
        if(Blocks[i][j]!=NULL){
          delete Blocks[i][j];
          update_block(Blocks[i][j],NULL,i,j);
        }
      delete[] Blocks[i];
      delete[] Stones[i];
    }
    delete[] Blocks;
    delete[] Stones;    
}

// RECURSIVE ELIMINATING (DFS)
void StateGo::eliminate_block(int *block,INDEX i,INDEX j)
{
    Blocks[i][j]=NULL;
#ifdef JAPANESE
    if(Stones[i][j]==Black)
        captured_b++;
    else
        captured_w++;
#endif
    Stones[i][j]=Empty;
    if(!ko_flag){
      ko_flag=true;
      ko_unique=true;
      koi=i;
      koj=j;
    }else{
      ko_unique=false;
    }
    if(i>0)
      if(Blocks[i-1][j]==block)//If same block, propagate.
        eliminate_block(block,i-1,j);
      else
        if(Stones[i-1][j]!=Empty)//Add a free adjacency.
          *Blocks[i-1][j]+=1;
    if(j>0)
      if(Blocks[i][j-1]==block)//If same block, propagate.
        eliminate_block(block,i,j-1);
      else
        if(Stones[i][j-1]!=Empty)//Add a free adjacency.
          *Blocks[i][j-1]+=1;
    if(j<_size-1)
      if(Blocks[i][j+1]==block)//If same block, propagate.
        eliminate_block(block,i,j+1);
      else
        if(Stones[i][j+1]!=Empty)//Add a free adjacency.
          *Blocks[i][j+1]+=1;
    if(i<_size-1)
      if(Blocks[i+1][j]==block)//If same block, propagate.
        eliminate_block(block,i+1,j);
      else
        if(Stones[i+1][j]!=Empty)//Add a free adjacency.
          *Blocks[i+1][j]+=1;
}

/* ITERACTIVE ELIMINATING (BFS)
#define POSI(i,j) ((i)*MAX_BOARD+(j))
#define I(p)      p/MAX_BOARD       
#define J(p)      p%MAX_BOARD

inline void StateGo::eliminate_block(int *block,INDEX i,INDEX j)
{

    Blocks[i][j]=NULL;
    Stones[i][j]=Empty;
    std::queue<int> list;
    list.push(POSI(i,j));
    int pos;
    while(!list.empty()){
    pos=list.front();
    list.pop();
    i=I(pos);
    j=J(pos);
#ifdef JAPANESE
    if(Stones[i][j]==Black)
        captured_b++;
    else
        captured_w++;
#endif
    if(!ko_flag){
      ko_flag=true;
      ko_unique=true;
      koi=i;
      koj=j;
    }else{
      ko_unique=false;
    }
    if(i>0)
      if(Blocks[i-1][j]==block){//If same block, propagate.
        Blocks[i-1][j]=NULL;
        Stones[i-1][j]=Empty;
        list.push(POSI(i-1,j));
      }
      else
        if(Stones[i-1][j]!=Empty)//Add a free adjacency.
          *Blocks[i-1][j]+=1;
    if(j>0)
      if(Blocks[i][j-1]==block){//If same block, propagate.
        Blocks[i][j-1]=NULL;
        Stones[i][j-1]=Empty;
        list.push(POSI(i,j-1));
      }
      else
        if(Stones[i][j-1]!=Empty)//Add a free adjacency.
          *Blocks[i][j-1]+=1;
    if(j<_size-1)
      if(Blocks[i][j+1]==block){//If same block, propagate.
        Blocks[i][j+1]=NULL;
        Stones[i][j+1]=Empty;
        list.push(POSI(i,j+1));
      }
      else
        if(Stones[i][j+1]!=Empty)//Add a free adjacency.
          *Blocks[i][j+1]+=1;
    if(i<_size-1)
      if(Blocks[i+1][j]==block){//If same block, propagate.
        Blocks[i+1][j]=NULL;
        Stones[i+1][j]=Empty;
        list.push(POSI(i+1,j));
      }
      else
        if(Stones[i+1][j]!=Empty)//Add a free adjacency.
          *Blocks[i+1][j]+=1;
    }
}
*/

// RECURSIVE UPDATING (DFS)
void StateGo::update_block(int *block,int *new_block,INDEX i,INDEX j)
{
    Blocks[i][j]=new_block;
    if(i>0)
      if(Blocks[i-1][j]==block)//If same block, propagate.
        update_block(block,new_block,i-1,j);
    if(j>0)
      if(Blocks[i][j-1]==block)//If same block, propagate.
        update_block(block,new_block,i,j-1);
    if(j<_size-1)
      if(Blocks[i][j+1]==block)//If same block, propagate.
        update_block(block,new_block,i,j+1);
    if(i<_size-1)
      if(Blocks[i+1][j]==block)//If same block, propagate.
        update_block(block,new_block,i+1,j);
}
/*
// ITERACTIVE UPDATING (BFS)
inline void StateGo::update_block(int *block,int *new_block,INDEX i,INDEX j)
{
    std::queue<int> list;
    list.push(POSI(i,j));
    int pos;
    Blocks[i][j]=new_block;
    while(!list.empty()){
    pos=list.front();
    list.pop();
    i=I(pos);
    j=J(pos);
    if(i>0)
      if(Blocks[i-1][j]==block){//If same block, propagate.
        Blocks[i-1][j]=new_block;
        list.push(POSI(i-1,j));
      }
    if(j>0)
      if(Blocks[i][j-1]==block){//If same block, propagate.
        Blocks[i][j-1]=new_block;
        list.push(POSI(i,j-1));
      }
    if(j<_size-1)
      if(Blocks[i][j+1]==block){//If same block, propagate.
        Blocks[i][j+1]=new_block;
        list.push(POSI(i,j+1));
      }
    if(i<_size-1)
      if(Blocks[i+1][j]==block){//If same block, propagate.
        Blocks[i+1][j]=new_block;
        list.push(POSI(i+1,j));
      }
    }
}
*/

unsigned int StateGo::count_area(bool **visited,INDEX i,INDEX j)
{
    
    unsigned int res=1,v;
    visited[i][j]=true;
    if(i>0)
      switch(Stones[i-1][j]){
        case Black: res|=FLAG_B;break;
        case White: res|=FLAG_W;break;
        default: if(!visited[i-1][j]){
                    v=count_area(visited,i-1,j);
                    res = (v | (res & FLAG_BOTH)) + (res & NO_FLAG);
                 }
      }
    if(j>0)
      switch(Stones[i][j-1]){
        case Black: res|=FLAG_B;break;
        case White: res|=FLAG_W;break;
        default: if(!visited[i][j-1]){
                    v=count_area(visited,i,j-1);
                    res = (v | (res & FLAG_BOTH)) + (res & NO_FLAG);
                 }
      }
    if(j<_size-1)
      switch(Stones[i][j+1]){
        case Black: res|=FLAG_B;break;
        case White: res|=FLAG_W;break;
        default: if(!visited[i][j+1]){
                    v=count_area(visited,i,j+1);
                    res = (v | (res & FLAG_BOTH)) + (res & NO_FLAG);
                 }
      }
    if(i<_size-1)
      switch(Stones[i+1][j]){
        case Black: res|=FLAG_B;break;
        case White: res|=FLAG_W;break;
        default: if(!visited[i+1][j]){
                    v=count_area(visited,i+1,j);
                    res = (v | (res & FLAG_BOTH)) + (res & NO_FLAG);
                 }
      }
    return res;
}

inline void StateGo::get_possible_moves(std::vector<DataGo>& v)
{
    if(pass==2)
        return;
    for(INDEX i=0;i<_size;i++)
        for(INDEX j=0;j<_size;j++)
            if(Stones[i][j]==Empty && no_ko_nor_suicide(i,j,turn))
                v.push_back({turn,i,j});
    v.push_back(PASS(turn));
}

inline ValGo StateGo::get_final_value()
{
    float res = final_value();
    if(res<0)
        return Black;
    else if(res>0)
        return White;
    return Empty;
}

float StateGo::get_final_score()
{
    return final_value();
}

inline float StateGo::final_value()
{
    if(pass<2)
        return Empty;
    bool **visited= new bool*[_size];
    for(int i=0;i<_size;i++){
        visited[i]= new bool[_size];
        for(int j=0;j<_size;j++)
            visited[i][j]=false;
    }
    unsigned int res,numb=0,numw=0;
    float countb=0,countw=0;
    for(int i=0;i<_size;i++)
        for(int j=0;j<_size;j++)
            if(Stones[i][j]==Empty && !visited[i][j]){
                res=count_area(visited,i,j);
                if((res & FLAG_B) && (res & FLAG_W))
                    continue;
                if(res & FLAG_B)
                    numb+=(res & NO_FLAG);
                if(res & FLAG_W)
                    numw+=(res & NO_FLAG);
            }
#ifndef JAPANESE
             else if(Stones[i][j]==White)
                countw++;
             else if(Stones[i][j]==Black)
                countb++;
#endif
    for(int i=0;i<_size;i++){
        delete[] visited[i];
    }
    delete[] visited;
#ifdef JAPANESE
    countb=float(numb)-float(captured_b);
    countw=float(numw)+_komi-float(captured_w);
#else
    countb+=float(numb);
    countw+=float(numw)+_komi;
#endif
    return countw-countb; 
}

inline void StateGo::apply(DataGo d)
{
    assert(d.player == turn);
    ko_unique=false;
    ko_flag=false;
    if(IS_PASS(d)){
        assert(pass<2);
        pass++;
    }
    else
    {
        assert(d.i>=0 && d.i<_size);
        assert(d.j>=0 && d.j<_size);
        assert(Stones[d.i][d.j] == Empty);
        pass=0;
        int *adj = new int;
        *adj = 0;
        Blocks[d.i][d.j] = adj;
        Stones[d.i][d.j] = turn;
        int i=d.i,j=d.j;
        //Reduce adj of adjacent blocks.
        //Count actual adj.
        //Delete blocks of opponent with adjacency equal to zero.
        if(i>0)
          if(Stones[i-1][j]==Empty)
            (*adj)++;
          else
            if(! --(*Blocks[i-1][j]))//if no free adjacency.
              if(Stones[i-1][j]!=d.player){
                delete Blocks[i-1][j];
                eliminate_block(Blocks[i-1][j],i-1,j);
              }
        if(j>0)
          if(Stones[i][j-1]==Empty)
            (*adj)++;
          else
            if(! --(*Blocks[i][j-1]))//if no free adjacency.
              if(Stones[i][j-1]!=d.player){
                delete Blocks[i][j-1];
                eliminate_block(Blocks[i][j-1],i,j-1);
              }
        if(j<_size-1)
          if(Stones[i][j+1]==Empty)
            (*adj)++;
          else
            if(! --(*Blocks[i][j+1]))//if no free adjacency.
              if(Stones[i][j+1]!=d.player){
                delete Blocks[i][j+1];
                eliminate_block(Blocks[i][j+1],i,j+1);
              }
        if(i<_size-1)
          if(Stones[i+1][j]==Empty)
            (*adj)++;
          else
            if(! --(*Blocks[i+1][j]))//if no free adjacency.
              if(Stones[i+1][j]!=d.player){
                delete Blocks[i+1][j];
                eliminate_block(Blocks[i+1][j],i+1,j);
              }

        //Try to join blocks of same color.
        bool first=true;
        if(i>0)
          if(Stones[i-1][j]==d.player && Blocks[i-1][j]!=Blocks[i][j])
            if(first){
              *Blocks[i-1][j]+=*adj;
              delete adj;
              Blocks[i][j]=Blocks[i-1][j];
              first=false;
            }
            else{
              *Blocks[i][j]+=*Blocks[i-1][j];
              delete Blocks[i-1][j];
              update_block(Blocks[i-1][j],Blocks[i][j],i-1,j);
            }
        if(j>0)
          if(Stones[i][j-1]==d.player && Blocks[i][j-1]!=Blocks[i][j])
            if(first){
              *Blocks[i][j-1]+=*adj;
              delete adj;
              Blocks[i][j]=Blocks[i][j-1];
              first=false;
            }
            else{
              *Blocks[i][j]+=*Blocks[i][j-1];
              delete Blocks[i][j-1];
              update_block(Blocks[i][j-1],Blocks[i][j],i,j-1);
            }
        if(j<_size-1)
          if(Stones[i][j+1]==d.player && Blocks[i][j+1]!=Blocks[i][j])
            if(first){
              *Blocks[i][j+1]+=*adj;
              delete adj;
              Blocks[i][j]=Blocks[i][j+1];
              first=false;
            }
            else{
              *Blocks[i][j]+=*Blocks[i][j+1];
              delete Blocks[i][j+1];
              update_block(Blocks[i][j+1],Blocks[i][j],i,j+1);
            }
        if(i<_size-1)
          if(Stones[i+1][j]==d.player && Blocks[i+1][j]!=Blocks[i][j])
            if(first){
              *Blocks[i+1][j]+=*adj;
              delete adj;
              Blocks[i][j]=Blocks[i+1][j];
              first=false;
            }
            else{
              *Blocks[i][j]+=*Blocks[i+1][j];
              delete Blocks[i+1][j];
              update_block(Blocks[i+1][j],Blocks[i][j],i+1,j);
            }

    }
    turn=CHANGE_PLAYER(turn);
}

void StateGo::show(FILE *output){
    char c;
    fprintf(output,"   ");
    for(int i=0;i<_size;i++)
        fprintf(output," %c",'A'+i+(i>7));
    for(int i=_size-1;i>=0;i--){
        fprintf(output,"\n%2d ",i+1);
        for(int j=0;j<_size;j++){
            switch(Stones[i][j]){
                case Empty: c='.';break;
                case White: c='O';break;
                case Black: c='X';break;
            }
            fprintf(output," %c",c);
        }
    }
}

void StateGo::show(){
    show(stdout);
}

inline bool StateGo::no_ko_nor_suicide(INDEX i,INDEX j,Player p)
{
    //Check Ko situation.
    if(ko_flag && ko_unique && koi==i && koj==j)
        return false;

    if(i>0 && Stones[i-1][j]==Empty)
        return true;
    if(j>0 && Stones[i][j-1]==Empty)
        return true;
    if(j<_size-1 && Stones[i][j+1]==Empty)
        return true;
    if(i<_size-1 && Stones[i+1][j]==Empty)
        return true;

    //Check if free block near position.
    int sum=0,*b[4]={NULL,NULL,NULL,NULL},c=0;
    if(i>0 && Stones[i-1][j]==p){
      sum+=*Blocks[i-1][j];
      b[c]=Blocks[i-1][j];
      c++;
      sum-=1;
    }
    if(j>0 && Stones[i][j-1]==p){
      if(Blocks[i][j-1]!=b[0]){
        sum+=*Blocks[i][j-1];
        b[c]=Blocks[i][j-1];
        c++;
      }
      sum-=1;
    }
    if(j<_size-1 && Stones[i][j+1]==p){
      if(Blocks[i][j+1]!=b[0] && Blocks[i][j+1]!=b[1]){
        sum+=*Blocks[i][j+1];
        b[c]=Blocks[i][j+1];
        c++;
      }
      sum-=1;
    }
    if(i<_size-1 && Stones[i+1][j]==p){
      if(Blocks[i+1][j]!=b[0] && Blocks[i+1][j]!=b[1] && Blocks[i+1][j]!=b[2]){
        sum+=*Blocks[i+1][j];
        b[c]=Blocks[i+1][j];
        c++;
      }
      sum-=1;
    }

    if(sum!=0)
        return true;

    b[0]=NULL;
    b[1]=NULL;
    b[2]=NULL;
    b[3]=NULL;
    c=0;
    int sumb[4]={0,0,0,0};

    //Check if some opponent block will be removed.
    if(i>0)
      if(Stones[i-1][j]!=p){
        sumb[c]+=(*Blocks[i-1][j])-1;
        b[c]=Blocks[i-1][j];
        c++;
      }
    if(j>0)
      if(Stones[i][j-1]!=p){
        if(Blocks[i][j-1]==b[0])
          sumb[0]-=1;
        else{
          sumb[c]=(*Blocks[i][j-1])-1;
          b[c]=Blocks[i][j-1];
          c++;
        }
      }
    if(j<_size-1)
      if(Stones[i][j+1]!=p){
        if(Blocks[i][j+1]==b[0])
          sumb[0]-=1;
        else if(Blocks[i][j+1]==b[1])
          sumb[1]-=1;
        else {
          sumb[c]=(*Blocks[i][j+1])-1;
          b[c]=Blocks[i][j+1];
          c++;
        }
      }
    if(i<_size-1)
      if(Stones[i+1][j]!=p){
        if(Blocks[i+1][j]==b[0])
          sumb[0]-=1;
        else if(Blocks[i+1][j]==b[1])
          sumb[1]-=1;
        else if(Blocks[i+1][j]==b[2])
          sumb[2]-=1;
        else{
            sumb[c]=(*Blocks[i+1][j])-1;
            b[c]=Blocks[i+1][j];
            c++;
        }
      }

    for(int k=0;k<c;k++)
      if(sumb[k]==0)
        return true;

    return false;
}

bool StateGo::valid_move(DataGo d)
{
    if(d.player!=turn)
      return false;
    if(IS_PASS(d))
      return pass<2; 
    if(Stones[d.i][d.j]!=Empty)
      return false;
    if(no_ko_nor_suicide(d.i,d.j,d.player))
      return true;
    return false;
}

