//
//  main.cpp
//  et-quadtree
//
//  Created by etond on 15/3/2.
//  Copyright (c) 2015年 etond. All rights reserved.
//

#include <iostream>
#include <vector>
#include <list>


/**
 * (x,y) 左下角
 *
 */
class Rectangle
{
public:
    Rectangle(int _x, int _y, int _width, int _height)
    :x(_x)
    ,y(_y)
    ,width(_width)
    ,height(_height)
    {
        
    }
    
    std::list<Rectangle> split(int _x, int _y)
    {
        std::list<Rectangle> array;
        
        if (_x > x && _x < x + width) {
            array.push_back(Rectangle(_x, y, width - (_x - x), height));
            array.push_back(Rectangle(x, y, _x - x, height));
        } else {
            array.push_back(*this);
        }
            
        if (_y > y && _y < y + height) {
            int size = (int)array.size();
            for (int i = 0; i < size; i++) {
                Rectangle rec = array.front();
                array.pop_front();
                
                array.push_back(Rectangle(rec.x, rec.y, rec.width, rec.height - (_y - rec.y)));
                array.push_back(Rectangle(rec.x, _y, rec.width, _y - rec.y));
            }
        }
        
        return array;
    }
    
    int x, y;
    int width, height;
};

class Node
{
public:
    Rectangle  _rec;
    Rectangle getRect() { return _rec; }
};


//template <class T>
class QuardTree
{
public:
    static QuardTree* create(Rectangle rect)
    {
        if (ROOT == NULL) {
            ROOT = new QuardTree(rect, 0);
        }
        
        return ROOT;
    }
    
    
    
    QuardTree(Rectangle bound, int level)
        :_bound(bound)
        ,_level(level)
        ,_parent(NULL)
        ,_pos(INVALID_POS)
        ,_objects(NULL) {
            for (int i = 0; i < 4; i++) {
                _nodes[i] = NULL;
            }
    }
    
    void insert(Node *node) {
        Rectangle rec = node->getRect();
        if (_nodes[0] != NULL) {
            int idx = getIndex(rec);
            
            if (idx >= 0) {
                _nodes[idx]->insert(node);
            }
            return ;
        }
        
        _objects.push_back(node);
        
        if (_objects.size() > MAX_OBJECTS && _level < MAX_LEVEL) {
            split();
            
            std::list<Node*>::iterator it = _objects.begin();
            while( it != _objects.end() ) {
                int idx = getIndex((*it)->getRect());
                
                if (idx >= 0) {
                    _nodes[idx]->insert(*it);
                    it = _objects.erase(it);
                } else {
                    it++;
                }
            }
        }
    }
    
    void retrive(std::vector<std::list<Node*> > &array, Rectangle rect) {
        
        array.push_back(_objects);
        
        if (_nodes[0] != NULL) {
            
            int idx = getIndex(rect);
            if (idx >= 0) {
                _nodes[idx]->retrive(array, rect);
            } else {
                std::list<Rectangle> l;
                
                // NEED FIX
                if (idx == INVALID_POS) {
                    l = rect.split(_bound.x + _bound.width / 2, _bound.y + _bound.height / 2);
                } else if (idx == -1 || idx == -2) {
                    l = rect.split(_bound.x + _bound.width / 2, _bound.y - _bound.height / 2);
                } else {
                    l = rect.split(_bound.x - _bound.width / 2, _bound.y + _bound.height / 2);
                }
                
                std::list<Rectangle>::iterator it = l.begin();
                for (; it != l.end(); it++) {
                    int nidx = getIndex(*it);
                    _nodes[nidx]->retrive(array, *it);
                }
            }
        }
    }
    
    void update(QuardTree *node)
    {
        if (node == NULL) {
            node = this;
        }
        
        std::list<Node*>::iterator it;
        for (it = node->_objects.begin(); it != node->_objects.end(); ) {
            Rectangle rect = (*it)->getRect();
            
            if (!isInbound(rect, node->_bound)) {
                it = node->_objects.erase(it);
                node->insert(*it);
            } else if (node->_nodes[0] == NULL) {
                int idx = node->getIndex((*it)->getRect());
                node->_nodes[idx]->insert((*it));
            }
            it++;
        }
        
        
        for (int i = 0; i < 4; i++) {
            node->_nodes[i]->update(NULL);
        }
    }
    
    void clear(QuardTree *node)
    {
        if (node == NULL) {
            node = this;
        }
        
        if (node->_nodes[0] != NULL) {
            for (int i = 0; i < 4; i++) {
                node->_nodes[i]->clear(NULL);
                delete node->_nodes[i];
                node->_nodes[i] = NULL;
            }
        }
    }
    
    
private:
    
    bool isInbound(Rectangle rect, Rectangle bounds)
    {
        return rect.x >= bounds.x
        && rect.x + bounds.width <= bounds.x + bounds.width
        && rect.y >= bounds.y
        && rect.y + rect.height <= bounds.y + bounds.height;
        
    }
    
    /**
     *  获得位置信息
     *
     *  |     |     |
     *  |  1 (1) 0  |
     *  |     |     |
     *  |-(3)-*-(4)-|
     *  |     |     |
     *  |  2 (2) 3  |
     *  |     |     |
     *
     *  @param rect
     *
     *  @return
     */
    int getIndex(Rectangle rect) {
        bool isTop = rect.y > _bound.y + _bound.height / 2;
        bool isButtom= rect.y + rect.height <= _bound.y + _bound.height / 2;
        bool isLeft = rect.x + rect.width <= _bound.x + _bound.width / 2;
        bool isRight = rect.x > _bound.x + _bound.width / 2;
        
        if (isTop) {
            if (isRight) {          /* 第1象限 */
                return 0;
            } else if (isRight) {   /* 第2象限 */
                return 1;
            } else {                /* (1) */
                return -1;
            }
        } else if (isButtom) {
            if (isLeft) {           /* 第3象限 */
                return 2;
            } else if (isRight) {   /* 第4象限 */
                return 3;
            } else {                /* (2) */
                return -2;
            }
        } else {
            if (isLeft) {           /* (3) */
                return -3;
            } else if (isRight) {   /* (4) */
                return -4;
            } else {                /* 中心位置 */
                return INVALID_POS;
            }
        }
        return INVALID_POS;
    }
    
    void split()
    {
        int x = _bound.x;
        int y = _bound.y;
        int hwidth = _bound.width / 2;
        int hheight = _bound.height / 2;
        
        for (int i = 0; i < 4; i++) {
            _nodes[i]->_parent = this;
        }
        
        _nodes[0] = new QuardTree(Rectangle(x + hwidth, y          , hwidth, hheight), _level + 1);
        _nodes[1] = new QuardTree(Rectangle(x         , y          , hwidth, hheight), _level + 1);
        _nodes[2] = new QuardTree(Rectangle(x         , y + hheight, hwidth, hheight), _level + 1);
        _nodes[3] = new QuardTree(Rectangle(x + hwidth, y + hheight, hwidth, hheight), _level + 1);
    }
    
    int _pos;
    int _level;
    Rectangle _bound;
    QuardTree *_parent;
    QuardTree *_nodes[4];
    std::list<Node*> _objects;
    
    static QuardTree *ROOT;
    
    static int MAX_OBJECTS;
    static int MAX_LEVEL;
    static int INVALID_POS;
};

int QuardTree::INVALID_POS = -32;
int QuardTree::MAX_LEVEL = 5;
int QuardTree::MAX_OBJECTS = 10;
QuardTree* QuardTree::ROOT = NULL;



int main(int argc, const char * argv[])
{
    QuardTree *root = QuardTree::create(Rectangle(0, 0, 200, 200));
    
    root->clear(NULL);
    
    root->clear(NULL);
    
    return 0;
}
